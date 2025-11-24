#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Rendering/Renderer/ExRendererGetters.h"
#include "Input/Input.h"
#include "Input/InputEvents/InputEventHandler.h"
#include "Runtime/App.h"
#include "Runtime/AppEvents/AppEventsHandler.h"
#include "Configuration/ConfigurationFileManager.h"
#include <SDL.h>
#include <glm/glm.hpp>

Engine::Engine(){
    if(!ConfigurationFileManager::Load())
        ConfigurationFileManager::SaveCurrentState();

    ecsManager = std::make_shared<ECSManager>();
    gameLoop = std::make_unique<Gameloop>(ecsManager);

    *AppEventsHandler::onApplicationQuitHandler += [this](){
        StopEngine();
    };

    InputEventHandler::Create();
};

Engine::~Engine(){
    StopEngine();
};

void Engine::InitializeEngine(){
    StartEngine();
};

void Engine::RunLoop(){
    while(running)
    {   
        if(App::isPlaying)
        {
            gameLoop->ExecuteGameLoop();
        }
        else
        {
            gameLoop->Update();
        }
    }
};

void Engine::StartEngine(){
    running = true;

    gameLoop->Initialize();
    ExRenderer::Initialize(ecsManager);
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};

std::shared_ptr<ECSManager> Engine::GetECSManagerPtr(){
    return ecsManager;
};