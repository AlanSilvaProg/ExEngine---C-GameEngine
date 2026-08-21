#include "ScriptHotReloadManager.h"
#include "ScriptCompiler.h"
#include "../../Engine/Core/ECS/InternalRegistry/ComponentRegistry.h"
#include "../../Engine/Core/ECS/InternalRegistry/SystemRegistry.h"
#include "../../Engine/Core/ECS/Pool/EComponentSPoolManager.h"
#include "../../Engine/Core/Progress/ProcessTracker.h"
#include "../../Engine/Logger/Logger.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <regex>
#include <utility>

ScriptHotReloadManager::ScriptHotReloadManager(std::shared_ptr<ECSManager> ecsManager, std::filesystem::path projectPath)
    : ecsManager(ecsManager), projectPath(std::move(projectPath))
{
    scriptModulesPath = this->projectPath / "Library" / "ScriptModules";
    // Not FileManagement::CreateDirectory: that helper deliberately never reuses an existing path
    // (it creates "ScriptModules(1)" alongside it instead), which is right for user-facing asset
    // creation but wrong for an idempotent build cache directory that must be the same every launch.
    std::filesystem::create_directories(scriptModulesPath);

    workerThread = std::thread(&ScriptHotReloadManager::WorkerThreadFunction, this);
};

ScriptHotReloadManager::~ScriptHotReloadManager(){
    running = false;
    pendingCondition.notify_all();
    if(workerThread.joinable()) workerThread.join();

    // Without this, ComponentRegistry/SystemRegistry keep closures pointing at code from these
    // modules after they're dlclose'd/FreeLibrary'd below - dangling until something touches that
    // registry entry again (which, for statics destroyed at process exit, crashes on shutdown).
    for(auto& [scriptPathKey, module] : loadedModules)
    {
        UnregisterAndUnload(module);
    }
    loadedModules.clear();
};

void ScriptHotReloadManager::UnregisterAndUnload(LoadedModule& module){
    if(module.kind == ScriptKind::Component)
    {
        ComponentRegistry::Unregister(module.registryId);
    }
    else if(module.kind == ScriptKind::System && module.systemInstance != nullptr)
    {
        auto context = ecsManager->GetECSystemContext(module.systemContext);
        if(context != nullptr) context->Unregister(module.systemTypeIndex, module.systemInstance);

        ecsManager->DestroySystem(module.systemTypeIndex);
        SystemRegistry::Unregister(module.registryId);
        module.systemInstance = nullptr;
    }

    module.library.Unload();
};

void ScriptHotReloadManager::OnScriptFileEvent(const std::string& filePath){
    if(std::filesystem::path(filePath).extension() != ".hpp") return;

    {
        std::lock_guard<std::mutex> lock(pendingMutex);
        pendingJobs.push_back(filePath);
    }
    pendingCondition.notify_one();
};

void ScriptHotReloadManager::OnScriptFileDeleted(const std::string& filePath){
    if(std::filesystem::path(filePath).extension() != ".hpp") return;

    std::lock_guard<std::mutex> lock(pendingDeletionsMutex);
    pendingDeletions.push_back(filePath);
};

void ScriptHotReloadManager::ApplyDeletion(const std::string& filePath){
    auto it = loadedModules.find(filePath);
    if(it == loadedModules.end()) return;

    auto& module = it->second;

    if(module.kind == ScriptKind::Component)
    {
        ComponentRegistry::Unregister(module.registryId);
        orphanedModules.push_back(std::move(module));

        // Any System that #included this Component's .hpp got its full definition (and
        // REGISTER_COMPONENT) compiled into the System's own module, so that System keeps running
        // with a fully functional but now-orphaned copy of the type - the deletion above only ever
        // touches the Component's own module/registry entry, never the System's. Force those Systems
        // to recompile right now so the missing header fails loudly at the moment of deletion,
        // instead of lingering until some unrelated future edit trips over it.
        auto deletedPath = std::filesystem::path(filePath).lexically_normal().string();
        for(const auto& [systemPath, dependencies] : systemDependencies)
        {
            if(std::find(dependencies.begin(), dependencies.end(), deletedPath) == dependencies.end()) continue;

            Logger::LogError("Component '" + std::filesystem::path(filePath).filename().string() +
                "' was deleted but is still #included by system '" + std::filesystem::path(systemPath).filename().string() +
                "' - recompiling that system now to surface the dangling reference.");
            OnScriptFileEvent(systemPath);
        }
    }
    else if(module.kind == ScriptKind::System)
    {
        UnregisterAndUnload(module);
        systemDependencies.erase(filePath);
    }

    loadedModules.erase(it);
    Logger::Log("Script deleted, unregistered: " + std::filesystem::path(filePath).filename().string());
};

std::string ScriptHotReloadManager::GetScriptPathForSystem(const std::shared_ptr<ECSystem>& systemInstance) const{
    for(const auto& [scriptPathKey, module] : loadedModules)
    {
        if(module.kind == ScriptKind::System && module.systemInstance == systemInstance) return scriptPathKey;
    }

    return "";
};

void ScriptHotReloadManager::ScanAndCompileExistingScripts(){
    if(!std::filesystem::exists(projectPath)) return;

    for(const auto& entry : std::filesystem::recursive_directory_iterator(projectPath))
    {
        if(entry.is_directory()) continue;
        if(entry.path().extension() != ".hpp") continue;

        OnScriptFileEvent(entry.path().string());
    }
};

void ScriptHotReloadManager::WorkerThreadFunction(){
    while(true)
    {
        std::string scriptPathKey;
        {
            std::unique_lock<std::mutex> lock(pendingMutex);
            pendingCondition.wait(lock, [this](){ return !pendingJobs.empty() || !running; });
            if(!running && pendingJobs.empty()) return;

            scriptPathKey = pendingJobs.front();
            pendingJobs.pop_front();
        }

        {
            std::lock_guard<std::mutex> lock(currentJobMutex);
            currentlyCompilingPath = scriptPathKey;
        }

        auto outcome = CompileOne(scriptPathKey);

        {
            std::lock_guard<std::mutex> lock(currentJobMutex);
            currentlyCompilingPath.clear();
        }

        std::lock_guard<std::mutex> lock(completedMutex);
        completedOutcomes.push_back(std::move(outcome));
    }
};

ScriptHotReloadManager::CompileOutcome ScriptHotReloadManager::CompileOne(const std::string& scriptPathKey){
    CompileOutcome outcome;
    outcome.scriptPathKey = scriptPathKey;
    outcome.scriptPath = scriptPathKey;

    std::ifstream file(scriptPathKey);
    if(!file.is_open())
    {
        outcome.kind = ScriptKind::Unknown;
        outcome.success = false;
        return outcome;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    outcome.kind = DetectScriptKind(source);
    if(outcome.kind == ScriptKind::Unknown) return outcome; // not an ECS script - silently ignored by ApplyOutcome

    std::string idFieldName = (outcome.kind == ScriptKind::Component) ? "ComponentId" : "SystemId";
    if(!ExtractUnsignedIntField(source, idFieldName, outcome.registryId))
    {
        outcome.success = false;
        outcome.compilerOutput = "Missing or invalid '" + idFieldName + "' in script: " + scriptPathKey;
        return outcome;
    }

    if(outcome.kind == ScriptKind::System)
    {
        outcome.systemContext = ExtractSystemContext(source);
        outcome.includedScriptPaths = ExtractIncludedScriptPaths(scriptPathKey, source);
    }

    int revision = ++revisionByScript[scriptPathKey];
    std::string moduleName = std::filesystem::path(scriptPathKey).stem().string() + "_r" + std::to_string(revision);

    auto compileResult = ScriptCompiler::Compile(scriptPathKey, scriptModulesPath, moduleName);
    outcome.success = compileResult.success;
    outcome.compilerOutput = compileResult.compilerOutput;
    outcome.compiledLibraryPath = compileResult.outputLibraryPath;

    return outcome;
};

bool ScriptHotReloadManager::IsCompiling() const{
    if(!GetPendingAndActiveScriptPaths().empty()) return true;

    std::lock_guard<std::mutex> lock(completedMutex);
    return !completedOutcomes.empty();
};

std::vector<std::string> ScriptHotReloadManager::GetPendingAndActiveScriptPaths() const{
    std::vector<std::string> paths;
    {
        std::lock_guard<std::mutex> lock(pendingMutex);
        paths.assign(pendingJobs.begin(), pendingJobs.end());
    }
    {
        std::lock_guard<std::mutex> lock(currentJobMutex);
        if(!currentlyCompilingPath.empty()) paths.push_back(currentlyCompilingPath);
    }
    return paths;
};

void ScriptHotReloadManager::UpdateProcessTracking(){
    for(const auto& scriptPathKey : GetPendingAndActiveScriptPaths())
    {
        if(processIdsByScript.find(scriptPathKey) != processIdsByScript.end()) continue;

        auto description = "Compiling script: " + std::filesystem::path(scriptPathKey).filename().string();
        processIdsByScript[scriptPathKey] = ProcessTracker::BeginProcess(description);
    }
};

void ScriptHotReloadManager::EndScriptProcess(const std::string& scriptPathKey){
    auto it = processIdsByScript.find(scriptPathKey);
    if(it == processIdsByScript.end()) return;

    ProcessTracker::EndProcess(it->second);
    processIdsByScript.erase(it);
};

void ScriptHotReloadManager::Poll(){
    UpdateProcessTracking();

    std::vector<std::string> deletionsToApply;
    {
        std::lock_guard<std::mutex> lock(pendingDeletionsMutex);
        deletionsToApply.swap(pendingDeletions);
    }

    for(const auto& filePath : deletionsToApply)
    {
        ApplyDeletion(filePath);
    }

    std::vector<CompileOutcome> outcomesToApply;
    {
        std::lock_guard<std::mutex> lock(completedMutex);
        outcomesToApply.swap(completedOutcomes);
    }

    for(const auto& outcome : outcomesToApply)
    {
        EndScriptProcess(outcome.scriptPathKey);
        ApplyOutcome(outcome);
    }
};

void ScriptHotReloadManager::ApplyOutcome(const CompileOutcome& outcome){
    if(outcome.kind == ScriptKind::Unknown) return; // regular header, not a script - nothing to do

    if(!outcome.success)
    {
        Logger::LogError("Script compile failed (" + outcome.scriptPath.filename().string() + "):\n" + outcome.compilerOutput);
        return;
    }

    std::vector<std::pair<int, nlohmann::json>> savedComponentState;

    auto existing = loadedModules.find(outcome.scriptPathKey);
    if(existing != loadedModules.end())
    {
        auto& previous = existing->second;

        if(previous.kind == ScriptKind::Component)
        {
            auto componentId = previous.registryId;
            auto& pools = ecsManager->GetEntityComponentPools();

            if(componentId < pools.size() && pools[componentId] != nullptr)
            {
                auto poolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pools[componentId]);
                for(auto entityId : ecsManager->GetAliveEntities())
                {
                    if(!ecsManager->HasComponent(entityId, componentId)) continue;

                    auto instance = poolManager->GetComponent(entityId);
                    if(instance == nullptr) continue;

                    savedComponentState.emplace_back(entityId, instance->ToJson());
                    ecsManager->RemoveComponent(entityId, componentId);
                }

                ecsManager->Update(); // flush the pending removals before dlclose runs
            }
        }

        UnregisterAndUnload(previous);
        Logger::Log("Unloaded previous version of script: " + outcome.scriptPath.filename().string());
        loadedModules.erase(existing);
    }

    LoadedModule module;
    module.kind = outcome.kind;
    module.registryId = outcome.registryId;
    module.systemContext = outcome.systemContext;

    std::string loadError;
    if(!module.library.Load(outcome.compiledLibraryPath, loadError))
    {
        Logger::LogError("Failed to load compiled script (" + outcome.scriptPath.filename().string() + "): " + loadError);
        return;
    }

    if(outcome.kind == ScriptKind::Component)
    {
        for(const auto& [entityId, json] : savedComponentState)
        {
            auto entity = ecsManager->GetEntity(entityId);
            if(entity == nullptr) continue;

            auto factoryIt = ComponentRegistry::components.find(outcome.registryId);
            if(factoryIt == ComponentRegistry::components.end()) continue;

            factoryIt->second(entity);

            auto& pools = ecsManager->GetEntityComponentPools();
            if(outcome.registryId >= pools.size() || pools[outcome.registryId] == nullptr) continue;

            auto poolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pools[outcome.registryId]);
            auto instance = poolManager->GetComponent(entityId);
            if(instance != nullptr) instance->FromJson(json);
        }

        if(!savedComponentState.empty())
        {
            Logger::Log("Restored " + std::to_string(savedComponentState.size()) + " instance(s) after reloading: " + outcome.scriptPath.filename().string());
        }
    }
    else if(outcome.kind == ScriptKind::System)
    {
        auto factoryIt = SystemRegistry::systemFactories.find(outcome.registryId);
        if(factoryIt != SystemRegistry::systemFactories.end())
        {
            auto [typeIndex, systemInstance] = factoryIt->second(ecsManager);
            module.systemTypeIndex = typeIndex;
            module.systemInstance = systemInstance;
        }

        systemDependencies[outcome.scriptPathKey] = outcome.includedScriptPaths;
    }

    loadedModules.emplace(outcome.scriptPathKey, std::move(module));
    Logger::Log("Script compiled and loaded: " + outcome.scriptPath.filename().string());
};

ScriptHotReloadManager::ScriptKind ScriptHotReloadManager::DetectScriptKind(const std::string& source){
    if(source.find("public EComponentS<") != std::string::npos) return ScriptKind::Component;
    if(source.find("public CustomECSystem") != std::string::npos) return ScriptKind::System;
    return ScriptKind::Unknown;
};

bool ScriptHotReloadManager::ExtractUnsignedIntField(const std::string& source, const std::string& fieldName, unsigned int& outValue){
    std::regex pattern(fieldName + R"(\s*=\s*(\d+))");
    std::smatch match;
    if(!std::regex_search(source, match, pattern)) return false;

    outValue = static_cast<unsigned int>(std::stoul(match[1].str()));
    return true;
};

std::vector<std::string> ScriptHotReloadManager::ExtractIncludedScriptPaths(const std::filesystem::path& scriptPath, const std::string& source){
    std::vector<std::string> result;
    std::regex includePattern(R"reg(#include\s*"([^"]+)")reg");

    for(std::sregex_iterator it(source.begin(), source.end(), includePattern), end; it != end; ++it)
    {
        std::filesystem::path includedPath((*it)[1].str());
        auto resolved = includedPath.is_absolute() ? includedPath : scriptPath.parent_path() / includedPath;

        if(resolved.extension() != ".hpp") continue; // engine headers are .h; only project scripts matter here

        result.push_back(resolved.lexically_normal().string());
    }

    return result;
};

SystemContext ScriptHotReloadManager::ExtractSystemContext(const std::string& source){
    std::regex pattern(R"(SystemContext::(\w+))");
    std::smatch match;
    if(!std::regex_search(source, match, pattern)) return SystemContext::UPDATE;

    const std::string& name = match[1].str();
    if(name == "EARLY_UPDATE") return SystemContext::EARLY_UPDATE;
    if(name == "FIXED_UPDATE") return SystemContext::FIXED_UPDATE;
    if(name == "LATE_UPDATE") return SystemContext::LATE_UPDATE;
    if(name == "PRE_RENDER") return SystemContext::PRE_RENDER;
    if(name == "POST_RENDER") return SystemContext::POST_RENDER;
    return SystemContext::UPDATE;
};
