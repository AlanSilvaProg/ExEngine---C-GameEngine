#include "GameEarlyUpdateEventHandler.h"

std::unique_ptr<GameEarlyUpdateEventHandler> GameEarlyUpdateEventHandler::handler = nullptr;

void GameEarlyUpdateEventHandler::Create(){
    if(handler == nullptr)
    {
        handler = std::make_unique<GameEarlyUpdateEventHandler>();
    }
};