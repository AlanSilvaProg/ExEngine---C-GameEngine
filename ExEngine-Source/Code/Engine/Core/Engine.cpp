#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Rendering/Renderer/ExRendererGetters.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Rendering/Components/SpriteComponent.h"
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
            gameLoop->ExecuteGameLoop();
        else
            gameLoop->Update();
    }
};

void Engine::StartEngine(){
    running = true;

    gameLoop->Initialize();
    ExRenderer::Initialize(ecsManager);

    auto entity = ecsManager->CreateEntity("Tank");
    entity.AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(55,0,0), glm::vec3(1,1,1));
    std::string tankImageAddress = GetAssetsPath() / std::string("tank.png");
    entity.AddComponent<SpriteComponent>("tank-image", tankImageAddress, 0, 0, false, false);

    CreateBaseCamera();
};

void Engine::CreateBaseCamera(){
    auto cameraEntity = ecsManager->CreateEntity("Camera");
    cameraEntity.AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    cameraEntity.AddComponent<CameraComponent>(0);


    auto cameraEntitySecond = ecsManager->CreateEntity("CameraSecond");
    cameraEntitySecond.AddComponent<TransformComponent>(glm::vec3(-500,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    cameraEntitySecond.AddComponent<CameraComponent>(1);
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};

std::shared_ptr<ECSManager> Engine::GetECSManagerPtr(){
    return ecsManager;
};