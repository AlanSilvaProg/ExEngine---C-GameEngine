#include "ECSWorld.h"
#include "../../File/FileManagement.h"

bool ECSWorld::SaveCurrentState(){ return CreateOrSave(); };

ECSWorldInfo& ECSWorld::LoadState(){
    Destroy();
    LoadData();
    GenerateWorldEntities();
    return worldInformation;
}; 

void ECSWorld::Unload(){
    //ToDo ask for save if it has pending changes
    Destroy();
};

void ECSWorld::GenerateWorldEntities(){
    worldEntities.clear();
    for(auto entityInfo : worldInformation.entityContainer)
    {
        std::shared_ptr<EntityCS> entity = nullptr;
        CREATE_ENTITY(entity, entityInfo)
        AttachEntity(entity);
    }
};

void ECSWorld::LoadData(){
    auto exFilePath = worldFilePath.replace_extension(".exworld");
    bool loadedSuccessfully = false;

    if(std::filesystem::exists(exFilePath))
    {
        loadedSuccessfully = FileManagement::LoadFromJson(exFilePath, worldInformation);
    }

    if(!loadedSuccessfully)
    {
        if(!CreateOrSave())
        {
            Logger::LogError("Scene load has failed");
            return;
        }
    }

    Logger::Log("Scene " + worldInformation.name + " loaded successfully");
};

bool ECSWorld::CreateOrSave(){
    if(!std::filesystem::exists(worldFilePath.parent_path()))
    {
        return false;
    }

    worldInformation.entityContainer.clear();
    
    std::unordered_set<int> idsToRemove;

    std::erase_if(worldEntities, [&](std::shared_ptr<EntityCS> e) { return ecsManager->GetAliveEntities().contains(e->GetId()) == false; });

    for(auto worldEntity : worldEntities)
    {   
        EntityContainer container(worldEntity);
        worldInformation.entityContainer.push_back(container);
    }

    std::erase_if(worldInformation.entityContainer,
        [&](const EntityContainer& c)
        {
            return idsToRemove.contains(c.currentId);
        }
    );

    auto exFilePath = worldFilePath.replace_extension(".exworld");
    return FileManagement::SaveFile(exFilePath, worldInformation.ToJson().dump());
};

void ECSWorld::AttachEntity(std::shared_ptr<EntityCS>entity){
    worldEntities.push_back(entity);
};

void ECSWorld::DetachEntity(std::shared_ptr<EntityCS> entity){
    std::erase_if(worldEntities, [&](std::shared_ptr<EntityCS> e){
        if(e == nullptr) return false;
        return e->GetGuid() == entity->GetGuid();
    });
};

void ECSWorld::Destroy(){
    for(auto entityContainer :worldEntities)
    {
        entityContainer->KillImmediately();
    }
};