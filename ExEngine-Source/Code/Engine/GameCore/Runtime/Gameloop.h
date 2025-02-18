#pragma once 
#include <memory>
#include "../../Core/ECS/ECSManager.h"
#include "RuntimeEvent/GameUpdateEventHandler.h"

class Gameloop{
private:
    std::shared_ptr<ECSManager> ecsManager;

    bool enableUpdate = false;
    bool isRunning;
    float lastUpdate;

    void Stop();
    void ProcessInputPhase();
    void ProcessCollisionPhase();
public:
    Gameloop(std::shared_ptr<ECSManager> ecsManager) : ecsManager(ecsManager){
        //Creating event Handlers
        GameUpdateEventHandler::Create();
    }; 

    void Initialize();
    void ExecuteGameLoop();
    void Update();
    void ProcessRenderPhase();
};