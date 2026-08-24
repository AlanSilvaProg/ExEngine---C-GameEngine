#pragma once
#include "../EngineGetters.h"
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "ECSWorldInfo.h"
#include <string>
#include <memory>
#include <filesystem>
#include <vector>

class ECSWorld{
private:
    ECSWorldInfo worldInformation; //serializable 
    std::vector<std::shared_ptr<EntityCS>> worldEntities;
    std::shared_ptr<ECSManager> ecsManager;
    std::filesystem::path worldFilePath;
    bool isIncremental;

    std::shared_ptr<EntityCS> CreateEntity(EntityContainer& entityInfo);
    void ConfigureEntityByData(std::shared_ptr<EntityCS>& entity, EntityContainer& entityInfo);
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
    void Unload();
    std::filesystem::path& GetWorldPath() { return worldFilePath; };

    void AttachEntity(std::shared_ptr<EntityCS>entity);
    void DetachEntity(std::shared_ptr<EntityCS> entity);
};