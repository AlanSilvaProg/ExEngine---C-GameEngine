#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Rendering/Renderer/ExRendererGetters.h"
#include "Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "Rendering/Renderer/RendererEvent/ResolutionChangeEventHandler.h"
#include "CameraSystem/NoCameraEventHandler.h"
#include "Input/Input.h"
#include "Input/InputEvents/InputEventHandler.h"
#include "Runtime/App.h"
#include "Runtime/AppEvents/AppEventsHandler.h"
#include "GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "Configuration/ConfigurationFileManager.h"
#include "CollisionSystem/ExPhysicsEngine.h"
#include "Runtime/Settings/RuntimeSettings.h"
#include "Animation/AnimationManager.h"
#include <SDL.h>
#include <glm/glm.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

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
    NoCameraEventHandler::Create();
    InputEventHandler::Create();
    GameUpdateEventHandler::Create();
};

void Engine::RunLoop(){
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg([](void* enginePtr){
        auto* engine = static_cast<Engine*>(enginePtr);
        if(!engine->running)
        {
            emscripten_cancel_main_loop();
            return;
        }

        engine->gameLoop->ExecuteGameLoop();
    }, this, 0, 1);
#else
    while(running)
    {
        gameLoop->ExecuteGameLoop();
    }
#endif
};

void Engine::StartEngine(){
    running = true;

    gameLoop->Initialize();
    
    if(RuntimeSettings::GetPhysicsEnabled())
        ExPhysicsEngine::Initialize(ecsManager);

    ExRenderer::Initialize(ecsManager);
    AnimationManager::InitializeAnimationSystem();
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};

std::shared_ptr<ECSManager> Engine::GetECSManagerPtr(){
    return ecsManager;
};