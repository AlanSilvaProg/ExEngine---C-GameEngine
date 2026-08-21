#pragma once
#include "ClickSystem.h"
#include "../ECS/ECSManager.h"
#include <memory>

class ExPhysicsEngine{   
private:
    static std::shared_ptr<ECSManager> ecsManager;
    static std::shared_ptr<ClickSystem> clickSystem;
public:
    static void Initialize(std::shared_ptr<ECSManager> ecsManagerInstance);
    static void RunCollisionRoutine();
    static void Stop();
};