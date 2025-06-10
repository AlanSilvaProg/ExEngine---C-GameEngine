#pragma once
#include <memory>
#include "../GameCore/Runtime/Gameloop.h"
#include "Runtime/Settings/RuntimeSettings.h"
#include "ECS/ECSManager.h"

class Engine{
private:
    std::shared_ptr<ECSManager> ecsManager;
    std::unique_ptr<Gameloop> gameLoop;

    bool running;

    void StartEngine();
    void StopEngine();

    void CreateBaseCamera();
public:
    Engine();
    ~Engine();
    void InitializeEngine();
    void RunLoop();

    std::shared_ptr<ECSManager> GetECSManagerPtr();
};