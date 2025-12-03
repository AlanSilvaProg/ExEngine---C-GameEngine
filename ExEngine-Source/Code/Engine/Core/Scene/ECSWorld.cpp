#include "ECSWorld.h"
#include "../../File/FileManagement.h"

bool ECSWorld::SaveCurrentState(){ return CreateOrSave(); };

ECSWorldInfo& ECSWorld::LoadState(){
    Destroy();
    LoadData();
    GenerateWorldEntities();
    return worldInformation;
}; 

void ECSWorld::GenerateWorldEntities(){
    for(auto entityInfo : worldInformation.entityContainer)
    {
        auto entity = CREATE_ENTITY(entityInfo.name, entityInfo.internal);
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
    for(auto aliveEntityId : ecsManager->GetAliveEntities())
    {
        EntityContainer container(*ecsManager->GetEntity(aliveEntityId));
        worldInformation.entityContainer.push_back(container);
    }

    auto exFilePath = worldFilePath.replace_extension(".exworld");
    return FileManagement::SaveFile(exFilePath, worldInformation.ToJson().dump());
};

void ECSWorld::Destroy(){
    if(!isIncremental){
        ecsManager->DestroyAllEntitiesImmediately();
        return;
    }
    //ToDo unload specific world entities
};