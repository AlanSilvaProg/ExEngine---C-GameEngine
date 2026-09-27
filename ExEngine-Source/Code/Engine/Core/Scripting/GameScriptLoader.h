#pragma once
#include "DynamicLibrary.h"
#include "../ECS/ECSManager.h"
#include <filesystem>
#include <memory>
#include <vector>

// Loads every compiled script module in a shipped Standalone build's Scripts folder once at
// startup (Web links scripts into the binary instead). Loading only registers component/system
// factories; LoadAll() also instantiates each script System onto the live ECSManager.
class GameScriptLoader{
private:
    static std::vector<DynamicLibrary> loadedModules;
public:
    static void LoadAll(const std::filesystem::path& scriptsDirectory, std::shared_ptr<ECSManager> ecsManager);
};
