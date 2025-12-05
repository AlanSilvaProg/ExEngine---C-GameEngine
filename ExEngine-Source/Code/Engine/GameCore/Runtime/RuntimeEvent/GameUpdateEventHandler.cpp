#include "GameUpdateEventHandler.h"

std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::earlyHandler = nullptr;
std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::updateHandler = nullptr;
std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::fixedUpdateHandler = nullptr;
std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::lateHandler = nullptr;

void GameUpdateEventHandler::Create(){
    if(earlyHandler == nullptr)
    {
        earlyHandler = std::make_unique<GameUpdateEventHandler>();
    }

    if(updateHandler == nullptr)
    {
        updateHandler = std::make_unique<GameUpdateEventHandler>();
    }

    if(fixedUpdateHandler == nullptr)
    {
        fixedUpdateHandler = std::make_unique<GameUpdateEventHandler>();
    }
    
    if(lateHandler == nullptr)
    {
        lateHandler = std::make_unique<GameUpdateEventHandler>();
    }
};