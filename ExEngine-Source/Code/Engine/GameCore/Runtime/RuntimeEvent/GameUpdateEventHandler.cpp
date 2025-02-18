#include "GameUpdateEventHandler.h"

std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::earlyhandler = nullptr;
std::unique_ptr<GameUpdateEventHandler> GameUpdateEventHandler::latehandler = nullptr;

void GameUpdateEventHandler::Create(){
    if(earlyhandler == nullptr)
    {
        earlyhandler = std::make_unique<GameUpdateEventHandler>();
    }
    
    if(latehandler == nullptr)
    {
        latehandler = std::make_unique<GameUpdateEventHandler>();
    }
};