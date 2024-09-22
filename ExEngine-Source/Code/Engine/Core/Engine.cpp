#include <SDL.h>
#include "Engine.h" 

Engine::Engine(){
    gameLoop = std::make_unique<Gameloop>();
    ResetGameState();
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
};

void Engine::StopEngine(){
    running = false;
    ResetGameState();
};

void Engine::ResetGameState(){
    //ToDo - Create a load and save and read file system to configure the initial state of each scene
};

void Engine::ProcessRenderPhase(){

};