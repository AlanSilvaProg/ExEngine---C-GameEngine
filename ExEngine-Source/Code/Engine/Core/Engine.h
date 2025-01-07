#pragma once
#include <memory>
#include "../GameCore/Runtime/Gameloop.h"
#include "Runtime/Settings/RuntimeSettings.h"

class Engine{
private:
    std::unique_ptr<Gameloop> gameLoop;

    bool running;

    void StartEngine();
    void StopEngine();

    void GameloopSequence();
    void ProcessRenderPhase();
public:
    Engine();
    ~Engine();
    void InitializeEngine();
};