#pragma once
#include "../EngineGetters.h"
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "ECSWorldInfo.h"
#include <string>
#include <memory>
#include <filesystem>
#include <vector>

#ifndef CREATE_ENTITY
#define CREATE_ENTITY(entity, entityInfo)  \
    entity = ecsManager->CreateEntity(entityInfo.name, entityInfo.internal); \
    entity->RegenerateGuid(&entityInfo.guid);\
    const auto componentsPool = ecsManager->GetEntityComponentPools();\
\
    for (const auto& componentEntry : entityInfo.components)\
    {\
        int id = componentEntry["id"];\
        const auto& data = componentEntry["data"];\
\
        if (!ComponentRegistry::components.contains(id))\
        {\
            Logger::LogError("Component ID not registered: " + std::to_string(id));\
        }\
\
        if (ComponentRegistry::components[id])\
        {\
            ComponentRegistry::components[id](entity);\
        }\
\
        auto pool = componentsPool[id];\
\
        if(pool == nullptr)\
        {\
            continue;\
        }\
\
        auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pool);\
\
        if(castedPoolManager == nullptr) continue;\
\
        const auto entityId = entity->GetId();\
        const auto component = castedPoolManager->GetComponent(entityId);\
\
        if(component == nullptr) continue;\
        \
        if(!ecsManager->HasComponent(entityId, component->GetComponentId())) continue;\
        component->FromJson(componentEntry["data"]);\
    }
#endif

class ECSWorld{
private:
    ECSWorldInfo worldInformation; //serializable 
    std::vector<std::shared_ptr<EntityCS>> worldEntities;
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
    void Unload();
    std::filesystem::path& GetWorldPath() { return worldFilePath; };

    void AttachEntity(std::shared_ptr<EntityCS>entity);
    void DetachEntity(std::shared_ptr<EntityCS> entity);
};