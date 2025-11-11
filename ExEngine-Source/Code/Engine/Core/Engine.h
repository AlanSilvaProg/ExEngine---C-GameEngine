#pragma once
#include "../GameCore/Runtime/Gameloop.h"
#include "Runtime/Settings/RuntimeSettings.h"
#include "ECS/ECSManager.h"
#include <memory>
#include <string>
#include <filesystem>
#include <SDL.h>

class Engine{
private:
    std::shared_ptr<ECSManager> ecsManager;
    std::unique_ptr<Gameloop> gameLoop;

    bool running;

    void StartEngine();
    void StopEngine();

    void CreateBaseCamera();
public:
    Engine();
    ~Engine();
    void InitializeEngine();
    void RunLoop();

    std::shared_ptr<ECSManager> GetECSManagerPtr();

    inline static std::filesystem::path GetEnginePath(){
        auto basePath = SDL_GetBasePath();
        if (!basePath) return std::filesystem::current_path();

        std::filesystem::path path = basePath;
        SDL_free(basePath);
        return path;
    };
    
    inline static std::filesystem::path GetEngineAssetsPath() { return Engine::GetEnginePath() / std::string("Assets"); };
};