#pragma once
#include "DynamicLibrary.h"
#include "../../Engine/Core/ECS/ECSManager.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Drives compilation and hot-reload of user .hpp scripts (Components and Systems) into the live
// ECS. FileWatcher events arrive on a background thread and are only enqueued here; the actual
// compiler invocation runs on this manager's own worker thread (so a ~1-3s clang invocation never
// stalls the editor UI); the resulting dlopen/dlclose and ECS mutation only ever happen from
// Poll(), which the caller must invoke from the main thread once per frame.
class ScriptHotReloadManager{
public:
    ScriptHotReloadManager(std::shared_ptr<ECSManager> ecsManager, std::filesystem::path projectPath);
    ~ScriptHotReloadManager();

    // Thread-safe: safe to call directly from FileWatcher's callback thread.
    void OnScriptFileEvent(const std::string& filePath);

    // Main-thread only. Unregisters a deleted script's componentId/systemId so it stops showing up
    // as an option (e.g. in the Inspector's Add Component list). For a Component, the compiled
    // module is intentionally kept loaded (see orphanedModules) rather than unloaded here: entities
    // that already had the component still hold live instances whose vtable/destructor lives in that
    // module, so dlclose'ing it immediately would leave those instances dangling. The Inspector is
    // expected to show those as a "missing component" with a button to remove them.
    void OnScriptFileDeleted(const std::string& filePath);

    // Main-thread only. Applies every compile result finished since the last call.
    void Poll();

    // Returns the source .hpp path backing a live System instance, or empty if it isn't script-backed
    // (an internal engine system, or an ad-hoc CustomECSystem created without a script).
    std::string GetScriptPathForSystem(const std::shared_ptr<ECSystem>& systemInstance) const;

    // Compiles every .hpp script already present in the project tree. Call once after a project
    // is opened, so scripts written in a previous session are usable immediately.
    void ScanAndCompileExistingScripts();

private:
    enum class ScriptKind { Unknown, Component, System };

    struct LoadedModule{
        DynamicLibrary library;
        ScriptKind kind = ScriptKind::Unknown;
        unsigned int registryId = 0;
        SystemContext systemContext = SystemContext::UPDATE;
        std::type_index systemTypeIndex = std::type_index(typeid(void));
        std::shared_ptr<ECSystem> systemInstance;
    };

    struct CompileOutcome{
        std::string scriptPathKey;
        std::filesystem::path scriptPath;
        bool success = false;
        std::string compilerOutput;
        std::filesystem::path compiledLibraryPath;
        ScriptKind kind = ScriptKind::Unknown;
        unsigned int registryId = 0;
        SystemContext systemContext = SystemContext::UPDATE;
    };

    std::shared_ptr<ECSManager> ecsManager;
    std::filesystem::path projectPath;
    std::filesystem::path scriptModulesPath;

    std::thread workerThread;
    std::atomic<bool> running{true};

    mutable std::mutex pendingMutex;
    std::condition_variable pendingCondition;
    std::deque<std::string> pendingJobs;

    std::mutex completedMutex;
    std::vector<CompileOutcome> completedOutcomes;

    // Set by WorkerThreadFunction right before/after CompileOne runs, so the main thread can see
    // which job (if any) is currently being compiled - pendingJobs alone only holds the ones still
    // waiting, since the in-flight one has already been popped off of it.
    mutable std::mutex currentJobMutex;
    std::string currentlyCompilingPath;

    std::unordered_map<std::string, LoadedModule> loadedModules;
    std::unordered_map<std::string, int> revisionByScript;

    // ProcessTracker ids for scripts currently queued or compiling, keyed by scriptPathKey.
    // Main-thread only (populated/drained from Poll()), unlike everything above it.
    std::unordered_map<std::string, int> processIdsByScript;

    // Modules for deleted Component scripts, kept alive (never unloaded) for the rest of the editor
    // session so any pre-existing component instances of that type remain valid. See OnScriptFileDeleted.
    std::vector<LoadedModule> orphanedModules;

    void WorkerThreadFunction();
    CompileOutcome CompileOne(const std::string& scriptPathKey);
    void ApplyOutcome(const CompileOutcome& outcome);

    // Thread-safe: reads pendingJobs and currentlyCompilingPath, both guarded by their own mutex.
    std::vector<std::string> GetPendingAndActiveScriptPaths() const;

    // Main-thread only. Starts a ProcessTracker entry for every queued/in-flight script Poll() hasn't
    // seen yet, so the editor's progress window reflects compilation as soon as it's discovered
    // (usually within a frame of being enqueued), not just once it finishes.
    void UpdateProcessTracking();
    void EndScriptProcess(const std::string& scriptPathKey);

    // Removes module's entry from ComponentRegistry/SystemRegistry and dlclose's it. Must run
    // before the module is ever unloaded - the registry entries are closures pointing at code from
    // that module's translation unit, and become dangling the instant it's unloaded. Used both when
    // swapping in a recompiled version (ApplyOutcome) and when this manager itself is torn down.
    void UnregisterAndUnload(LoadedModule& module);

    static ScriptKind DetectScriptKind(const std::string& source);
    static bool ExtractUnsignedIntField(const std::string& source, const std::string& fieldName, unsigned int& outValue);
    static SystemContext ExtractSystemContext(const std::string& source);
};
