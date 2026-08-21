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

    // Thread-safe: safe to call directly from FileWatcher's callback thread. Only queues the path;
    // the actual unregister runs on the main thread from Poll() (see ApplyDeletion) so it can never
    // race the Inspector/ECSAdmin reading ComponentRegistry, or Poll()/ApplyOutcome mutating
    // loadedModules, on the main thread at the same time.
    void OnScriptFileDeleted(const std::string& filePath);

    // Main-thread only. Applies every compile result finished since the last call.
    void Poll();

    // Returns the source .hpp path backing a live System instance, or empty if it isn't script-backed
    // (an internal engine system, or an ad-hoc CustomECSystem created without a script).
    std::string GetScriptPathForSystem(const std::shared_ptr<ECSystem>& systemInstance) const;

    // Compiles every .hpp script already present in the project tree. Call once after a project
    // is opened, so scripts written in a previous session are usable immediately.
    void ScanAndCompileExistingScripts();

    // Thread-safe. True while any script is queued, actively compiling, or has finished compiling
    // but hasn't been applied to the live ECS by Poll() yet. Callers that need every project script
    // registered before proceeding (e.g. loading the last session's world only after
    // ScanAndCompileExistingScripts has fully landed) should hold off while this is true.
    bool IsCompiling() const;

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
        std::vector<std::string> includedScriptPaths; // System scripts only - see systemDependencies
    };

    std::shared_ptr<ECSManager> ecsManager;
    std::filesystem::path projectPath;
    std::filesystem::path scriptModulesPath;

    std::thread workerThread;
    std::atomic<bool> running{true};

    mutable std::mutex pendingMutex;
    std::condition_variable pendingCondition;
    std::deque<std::string> pendingJobs;

    mutable std::mutex completedMutex;
    std::vector<CompileOutcome> completedOutcomes;

    // Set by WorkerThreadFunction right before/after CompileOne runs, so the main thread can see
    // which job (if any) is currently being compiled - pendingJobs alone only holds the ones still
    // waiting, since the in-flight one has already been popped off of it.
    mutable std::mutex currentJobMutex;
    std::string currentlyCompilingPath;

    std::unordered_map<std::string, LoadedModule> loadedModules;
    std::unordered_map<std::string, int> revisionByScript;

    // Paths queued by OnScriptFileDeleted from whatever thread the FileWatcher calls back on;
    // drained on the main thread by Poll() -> ApplyDeletion.
    std::mutex pendingDeletionsMutex;
    std::vector<std::string> pendingDeletions;

    // System script path -> lexically-normalized paths of every project .hpp it #includes (as found
    // in its last successful compile). A System that #includes a Component's .hpp gets that
    // Component's full definition (and its REGISTER_COMPONENT) baked into the System's own compiled
    // module, so the reference survives independently of the Component's own module/script. This map
    // is what lets OnScriptFileDeleted notice "this System still embeds the Component you just
    // deleted" and react instead of leaving a silently-stale reference.
    std::unordered_map<std::string, std::vector<std::string>> systemDependencies;

    // ProcessTracker ids for scripts currently queued or compiling, keyed by scriptPathKey.
    // Main-thread only (populated/drained from Poll()), unlike everything above it.
    std::unordered_map<std::string, int> processIdsByScript;

    // Modules for deleted Component scripts, kept alive (never unloaded) for the rest of the editor
    // session so any pre-existing component instances of that type remain valid. See OnScriptFileDeleted.
    std::vector<LoadedModule> orphanedModules;

    void WorkerThreadFunction();
    CompileOutcome CompileOne(const std::string& scriptPathKey);
    void ApplyOutcome(const CompileOutcome& outcome);

    // Main-thread only (called from Poll()). Actual body of what used to run synchronously inside
    // OnScriptFileDeleted - see that method's comment for why it now only enqueues instead.
    void ApplyDeletion(const std::string& filePath);

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

    // Resolves every #include "..." in a System's source relative to the System's own directory
    // (matching how the compiler's quote-include lookup actually resolves them) and keeps the ones
    // pointing at another project .hpp - i.e. a Component or System script, as opposed to an engine
    // header like "Code/Engine/ExEngine.h" (.h, not .hpp, so it's filtered out naturally).
    static std::vector<std::string> ExtractIncludedScriptPaths(const std::filesystem::path& scriptPath, const std::string& source);
};
