#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Components/TransformComponent.h"
#include "Components/CameraComponent.h"
#include "Rendering/Components/SpriteComponent.h"
#include "Input/Input.h"
#include "Input/InputEvents/InputEventHandler.h"
#include "Runtime/App.h"
#include "Runtime/AppEvents/AppEventsHandler.h"
#include <SDL.h>
#include <glm/glm.hpp>

Engine::Engine(){
    ecsManager = std::make_shared<ECSManager>();
    gameLoop = std::make_unique<Gameloop>(ecsManager);

    *AppEventsHandler::onApplicationQuitHandler += [this](){
        StopEngine();
    };

    if(InputEventHandler::handler == nullptr)
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

    auto entity = ecsManager->CreateEntity();
    entity.AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(55,0,0), glm::vec3(1,1,1));
    entity.AddComponent<SpriteComponent>("tank-image", "/tank.png", 0, 0, false, false);

    auto cameraEntity = ecsManager->CreateEntity();
    cameraEntity.AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(55,0,0), glm::vec3(1,1,1));
    cameraEntity.AddComponent<CameraComponent>(glm::vec2(1,1), 0, glm::vec2(800,800));
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};