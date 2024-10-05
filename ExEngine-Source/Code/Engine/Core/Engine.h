#pragma once
#include <memory>
#include "Runtime/Gameloop.h"
#include "Runtime/Settings/RuntimeSettings.h"

class Engine{
private:
    std::unique_ptr<Gameloop> gameLoop;

    bool running;

    void StartEngine();
    void StopEngine();
    void ResetGameState();

    void GameloopSequence();
    //Game Engine Phases
    void ProcessRenderPhase();
public:
    Engine();
    ~Engine();
    void InitializeEngine();
};