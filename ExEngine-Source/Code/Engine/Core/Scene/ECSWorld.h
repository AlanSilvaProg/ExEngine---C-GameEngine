#pragma once
#include "../EngineGetters.h"
#include "../ECS/ECSManager.h"
#include "ECSWorldInfo.h"
#include <memory>
#include <filesystem>

#ifndef CREATE_ENTITY
#define CREATE_ENTITY(name, isInternal)  ecsManager->CreateEntity(name, isInternal); 
#endif

class ECSWorld{
private:
    ECSWorldInfo worldInformation; //serializable 
    std::shared_ptr<ECSManager> ecsManager;
    std::filesystem::path worldFilePath;
    bool isIncremental;

    void GenerateWorldEntities();
    void LoadData();
    bool CreateOrSave();
    void Destroy();
public:
    inline ECSWorld() { ecsManager = EngineGetters::GetEnginePtr()->GetECSManagerPtr(); worldInformation.name = "Default World"; };
    inline ECSWorld(std::filesystem::path worldFilePath) : worldFilePath(worldFilePath) { ecsManager = EngineGetters::GetEnginePtr()->GetECSManagerPtr(); };

    inline ECSWorldInfo& GetWorldInfo() { return worldInformation; };
    bool SaveCurrentState();
    ECSWorldInfo& LoadState(); //returns current loaded state
    std::filesystem::path& GetWorldPath() { return worldFilePath; };
};