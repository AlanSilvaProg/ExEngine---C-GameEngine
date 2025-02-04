#include "GameLateUpdateEventHandler.h"

std::unique_ptr<GameLateUpdateEventHandler> GameLateUpdateEventHandler::handler = nullptr;

void GameLateUpdateEventHandler::Create(){
    if(GameLateUpdateEventHandler::handler == nullptr)
    {
        handler = std::make_unique<GameLateUpdateEventHandler>();
    }
};