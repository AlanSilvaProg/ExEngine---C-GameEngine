#pragma once
#include "../EngineGetters.h"
#include "../ECS/ECSManager.h"
#include "ECSWorldInfo.h"
#include <memory>

#ifndef CREATE_ENTITY
#define CREATE_ENTITY(name)  ecsManager->CreateEntity(name); 
#endif

class ECSWorld{
private:
    ECSWorldInfo worldInformation; //serializable 
    std::shared_ptr<ECSManager> ecsManager;

    void GenerateWorldEntities();
    void ResetWorld();
public:
    inline ECSWorld() { ecsManager = EngineGetters::GetEnginePtr()->GetECSManagerPtr(); };

    inline ECSWorldInfo& GetWorldInfo() { return worldInformation; };
    bool SaveCurrentState();
    ECSWorldInfo& LoadState(); //returns current loaded state
};