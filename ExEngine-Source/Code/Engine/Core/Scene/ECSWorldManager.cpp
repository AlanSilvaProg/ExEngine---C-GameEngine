#include "ECSWorldManager.h"

std::shared_ptr<ECSWorld> ECSWorldManager::currentWorld = nullptr;
std::vector<ECSWorld> ECSWorldManager::incrementalWorlds;
std::map<std::filesystem::path, std::shared_ptr<ECSWorld>> ECSWorldManager::loadedECSWorlds;

//Main world
void ECSWorldManager::LoadWorld(std::filesystem::path worldFilePath){
    if(currentWorld != nullptr)
    {
        currentWorld->Unload();
    }

    if(auto loadedWorld = loadedECSWorlds.find(worldFilePath); loadedWorld != loadedECSWorlds.end()) 
        currentWorld = loadedWorld->second;
    else 
    {
        currentWorld = std::make_shared<ECSWorld>(worldFilePath);
        loadedECSWorlds.emplace(worldFilePath, currentWorld);
    }
    
    currentWorld->LoadState();
};

void ECSWorldManager::GenerateWorld(){
    currentWorld = std::make_shared<ECSWorld>();
};

void ECSWorldManager::OnWorldFileRenamed(const std::filesystem::path& oldPath, const std::filesystem::path& newPath){
    auto loadedWorld = loadedECSWorlds.find(oldPath);
    if(loadedWorld == loadedECSWorlds.end()) return;

    auto world = loadedWorld->second;
    loadedECSWorlds.erase(loadedWorld);

    world->GetWorldPath() = newPath;
    world->GetWorldInfo().name = newPath.stem().string();

    loadedECSWorlds.emplace(newPath, world);
};

//Incremental Worlds 
void ECSWorldManager::LoadIncrementalWorld(std::filesystem::path worldFilePath){

};

void ECSWorldManager::UnloadIncrementalWorld(const uint32_t worldHash){

};

void ECSWorldManager::UnloadIncrementalWorld(const uint8_t worldIndex){
    
};

void ECSWorldManager::UnloadIncrementalWorld(const std::string worldName){
    
};

//Getters
const ECSWorldInfo& ECSWorldManager::GetIncrementalWorld(const uint32_t worldHash){
    return currentWorld->GetWorldInfo();
};

const ECSWorldInfo& ECSWorldManager::GetIncrementalWorld(const uint8_t worldIndex){
    return currentWorld->GetWorldInfo();
};

const ECSWorldInfo& ECSWorldManager::GetIncrementalWorld(const std::string worldName){
    return currentWorld->GetWorldInfo();
};