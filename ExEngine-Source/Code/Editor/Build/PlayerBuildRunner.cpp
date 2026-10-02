#include "PlayerBuildRunner.h"
#include "../../Engine/Core/Progress/ProcessTracker.h"

PlayerBuildRunner::~PlayerBuildRunner(){
    if(workerThread.joinable()) workerThread.join();
};

void PlayerBuildRunner::Start(PlayerBuildKind kind, const std::filesystem::path& projectPath, const ProjectInfo& projectInfo, const std::filesystem::path& outputDir, bool devMode){
    if(running) return;

    if(workerThread.joinable()) workerThread.join(); // previous run already finished, just needs joining

    running = true;

    {
        std::lock_guard<std::mutex> lock(resultMutex);
        resultPending = false;
    }

    workerThread = std::thread([this, kind, projectPath, projectInfo, outputDir, devMode](){
        std::string error;
        bool success = false;

        BuildStepCallback onStep = [this](const std::string& description, int index, int total){
            ReportStep(description, index, total);
        };

        if(kind == PlayerBuildKind::Standalone)
            success = PlayerBuilder::BuildStandalone(projectPath, projectInfo, outputDir, devMode, onStep, error);
        else
            success = PlayerBuilder::BuildWeb(projectPath, projectInfo, outputDir, devMode, onStep, error);

        {
            std::lock_guard<std::mutex> lock(resultMutex);
            resultSuccess = success;
            resultMessage = success ? ("Build finished at: " + outputDir.string()) : error;
            resultPending = true;
        }

        running = false;
    });
};

void PlayerBuildRunner::ReportStep(const std::string& description, int index, int total){
    std::lock_guard<std::mutex> lock(stepMutex);
    stepDescription = description;
    stepIndex = index;
    stepTotal = total;
    stepChangedSinceLastPoll = true;
};

void PlayerBuildRunner::Poll(){
    std::string description;
    int index = 0;
    bool changed = false;

    {
        std::lock_guard<std::mutex> lock(stepMutex);
        if(stepChangedSinceLastPoll)
        {
            description = stepDescription;
            index = stepIndex;
            changed = true;
            stepChangedSinceLastPoll = false;
        }
    }

    if(changed && index != lastAppliedStepIndex)
    {
        if(currentProcessTrackerId != -1) ProcessTracker::EndProcess(currentProcessTrackerId);
        currentProcessTrackerId = ProcessTracker::BeginProcess(description);
        lastAppliedStepIndex = index;
    }

    if(!running && currentProcessTrackerId != -1)
    {
        ProcessTracker::EndProcess(currentProcessTrackerId);
        currentProcessTrackerId = -1;
        lastAppliedStepIndex = -1;
    }
};

bool PlayerBuildRunner::HasResult() const{
    std::lock_guard<std::mutex> lock(resultMutex);
    return resultPending;
};

bool PlayerBuildRunner::ConsumeResult(std::string& outMessage){
    std::lock_guard<std::mutex> lock(resultMutex);
    outMessage = resultMessage;
    bool success = resultSuccess;
    resultPending = false;
    return success;
};
