#pragma once
#include "PlayerBuilder.h"
#include "../../Engine/Core/Project/ProjectInfo.h"
#include <atomic>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>

enum class PlayerBuildKind { Standalone, Web };

// Runs a PlayerBuilder export on a background thread, relaying progress into ProcessTracker via
// Poll() so ProcessProgressWindow shows a build progress bar for free. Poll() must run once per
// frame from the main thread regardless of window visibility, or the progress relay freezes.
class PlayerBuildRunner{
public:
    ~PlayerBuildRunner();

    // No-op if a build is already running.
    void Start(PlayerBuildKind kind, const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode);

    // Main-thread only.
    void Poll();

    inline bool IsRunning() const { return running; };

    // True once a finished build's result hasn't been read yet. Main-thread only.
    bool HasResult() const;
    // Reads and clears the pending result, returning whether the build succeeded. Main-thread only.
    bool ConsumeResult(std::string& outMessage);

private:
    std::thread workerThread;
    std::atomic<bool> running{false};

    mutable std::mutex stepMutex;
    std::string stepDescription;
    int stepIndex = 0;
    int stepTotal = 0;
    bool stepChangedSinceLastPoll = false; // guarded by stepMutex

    mutable std::mutex resultMutex;
    bool resultSuccess = false;
    std::string resultMessage;
    bool resultPending = false;

    // Main-thread only, driving ProcessTracker.
    int lastAppliedStepIndex = -1;
    int currentProcessTrackerId = -1;

    // Thread-safe: called from the worker thread via PlayerBuilder's onStep callback.
    void ReportStep(const std::string& description, int index, int total);
};
