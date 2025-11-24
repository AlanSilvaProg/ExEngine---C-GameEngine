#pragma once
#include "ECSWorld.h"
#include <filesystem>
#include <memory>
#include <vector>
#include <map>
#include <cstdint>

class ECSWorldManager{
private: 
    static std::shared_ptr<ECSWorld> currentWorld;
    static std::vector<ECSWorld> incrementalWorlds;
    static std::map<std::filesystem::path, std::shared_ptr<ECSWorld>> loadedECSWorlds;
public:
    //Main world
    static inline void SaveCurrentWorld() { if(currentWorld != nullptr) currentWorld->SaveCurrentState(); };
    static inline void ReloadCurrentWorldState() { if(currentWorld != nullptr) currentWorld->LoadState(); };
    static void LoadWorld(std::filesystem::path worldFilePath);
    static void GenerateWorld();
    static inline const std::shared_ptr<ECSWorld> GetCurrentWorld() { return currentWorld; };

    //Incremental Worlds 
    static void LoadIncrementalWorld(std::filesystem::path worldFilePath);
    static void UnloadIncrementalWorld(const uint32_t worldHash);
    static void UnloadIncrementalWorld(const uint8_t worldIndex);
    static void UnloadIncrementalWorld(const std::string worldName);

    //Getters
    static inline const ECSWorldInfo& GetCurrentWorldInfo() { return currentWorld->GetWorldInfo(); };
    static const ECSWorldInfo& GetIncrementalWorld(const uint32_t worldHash);
    static const ECSWorldInfo& GetIncrementalWorld(const uint8_t worldIndex);
    static const ECSWorldInfo& GetIncrementalWorld(const std::string worldName);
    static inline const uint GetIncrementalWorldsLength() { return incrementalWorlds.size(); };

    //Helpers
    static inline const bool HasCurrentWorld() { return currentWorld != nullptr; };
    static inline const bool HasIncrementalWorlds(){ return incrementalWorlds.size() > 0; };
};