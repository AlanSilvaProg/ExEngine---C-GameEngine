#pragma once 
#include <memory>
#include "../../Core/ECS/ECSManager.h"

class Gameloop{
private:
    std::shared_ptr<ECSManager> ecsManager;

    bool enableUpdate = false;
    bool isRunning;
    float lastUpdate;

    void FixedUpdate();
    void Stop();

    void ProcessInputPhase();
    void ProcessCollisionPhase();
public:
    Gameloop(std::shared_ptr<ECSManager> ecsManager) : ecsManager(ecsManager){}; 

    void Initialize();
    void ExecuteGameLoop();
    void ProcessRenderPhase();
};