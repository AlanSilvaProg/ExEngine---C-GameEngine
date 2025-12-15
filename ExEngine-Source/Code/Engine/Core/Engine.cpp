#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Rendering/Renderer/ExRendererGetters.h"
#include "Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "Rendering/Renderer/RendererEvent/ResolutionChangeEventHandler.h"
#include "Input/Input.h"
#include "Input/InputEvents/InputEventHandler.h"
#include "Runtime/App.h"
#include "Runtime/AppEvents/AppEventsHandler.h"
#include "GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"

#include "Configuration/ConfigurationFileManager.h"
#include <SDL.h>
#include <glm/glm.hpp>

Engine::Engine(){
    if(!ConfigurationFileManager::Load())
        ConfigurationFileManager::SaveCurrentState();

    CreateEngineBaseEventHandlers();
    
    ecsManager = std::make_shared<ECSManager>();
    gameLoop = std::make_unique<Gameloop>(ecsManager);

    *AppEventsHandler::onApplicationQuitHandler += [this](){
        StopEngine();
    };
};

Engine::~Engine(){
    StopEngine();
};

void Engine::InitializeEngine(){
    StartEngine();
};

void Engine::CreateEngineBaseEventHandlers(){
    PreRenderEventHandler::Create();
    ResolutionChangeEventHandler::Create();
    InputEventHandler::Create();
    GameUpdateEventHandler::Create();
};

void Engine::RunLoop(){
    while(running)
    {   
        gameLoop->ExecuteGameLoop();
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