#include "GameScriptLoader.h"
#include "../ECS/InternalRegistry/SystemRegistry.h"
#include "../../Logger/Logger.h"

std::vector<DynamicLibrary> GameScriptLoader::loadedModules;

void GameScriptLoader::LoadAll(const std::filesystem::path& scriptsDirectory, std::shared_ptr<ECSManager> ecsManager){
    if(std::filesystem::exists(scriptsDirectory))
    {
#ifdef _WIN32
        const std::string libraryExtension = ".dll";
#elif defined(__APPLE__)
        const std::string libraryExtension = ".dylib";
#else
        const std::string libraryExtension = ".so";
#endif

        for(const auto& entry : std::filesystem::directory_iterator(scriptsDirectory)){
            if(entry.path().extension() != libraryExtension) continue;

            DynamicLibrary library;
            std::string loadError;

            if(!library.Load(entry.path(), loadError)){
                Logger::LogError("GameScriptLoader: failed to load " + entry.path().filename().string() + ": " + loadError);
                continue;
            }

            Logger::Log("GameScriptLoader: loaded script module " + entry.path().filename().string());
            loadedModules.push_back(std::move(library));
        }
    }

    // REGISTER_SYSTEM only ever registers a factory (see SystemRegistry.h) - every one that showed
    // up above (Standalone) or was already linked in (Web) still needs to actually be instantiated
    // and attached to the live ECSManager, exactly once, right here.
    for(const auto& [systemId, factory] : SystemRegistry::systemFactories){
        factory(ecsManager);
    }
};
