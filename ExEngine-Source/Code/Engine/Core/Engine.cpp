#include <SDL.h>
#include <glm/glm.hpp>
#include "Engine.h" 
#include "Rendering/Renderer/ExRenderer.h"
#include "Components/TransformComponent.h"
#include "Rendering/Components/SpriteComponent.h"
#include "Input/Input.h"

Engine::Engine(){
    ecsManager = std::make_shared<ECSManager>();
    gameLoop = std::make_unique<Gameloop>(ecsManager);
};

Engine::~Engine(){
    StopEngine();
};

void Engine::InitializeEngine(){
    StartEngine();

    while(running)
    {   
        gameLoop->ExecuteGameLoop();
        if(Input::GetButtonDown(SDLK_ESCAPE))
        {
            StopEngine();
        }
    }
};

void Engine::StartEngine(){
    running = true;

    gameLoop->Initialize();
    ExRenderer::Initialize(ecsManager);

    auto entity = ecsManager->CreateEntity();
    entity.AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(55,0,0), glm::vec3(1,1,1));
    entity.AddComponent<SpriteComponent>("tank-image", "/tank.png", 0, 0, false, false);
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};