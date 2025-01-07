#include <SDL.h>
#include "Engine.h" 
#include "../Logger/Logger.h"
#include "Rendering/Renderer/ExRenderer.h"

Engine::Engine(){
    gameLoop = std::make_unique<Gameloop>();
};

Engine::~Engine(){
    StopEngine();
};

void Engine::InitializeEngine(){
    StartEngine();
    while(running)
    {   
        gameLoop->ExecuteGameLoop();
    
        ProcessRenderPhase();
    }
};

void Engine::StartEngine(){
    running = true;
    gameLoop->Initialize();
    ExRenderer::Initialize();
};

void Engine::StopEngine(){
    running = false;
    ExRenderer::Quit();
};

void Engine::ProcessRenderPhase(){
    ExRenderer::RenderSequence();
};