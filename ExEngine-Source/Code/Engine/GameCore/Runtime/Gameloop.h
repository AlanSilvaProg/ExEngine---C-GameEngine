#pragma once 
#include <memory>
#include "../../Core/ECS/ECSManager.h"
#include "RuntimeEvent/GameUpdateEventHandler.h"

class Gameloop{
private:
    std::shared_ptr<ECSManager> ecsManager;

    bool enableUpdate = false;
    bool isRunning;

    void Stop();
    void ProcessInputPhase();
    void ProcessCollisionPhase();
public:
    Gameloop(std::shared_ptr<ECSManager> ecsManager) : ecsManager(ecsManager){ }; 

    void Initialize();
    void ExecuteGameLoop();
    void Update();
    void FixedUpdate();
    void ProcessRenderPhase();
};