#include "AppEventsHandler.h"

std::unique_ptr<AppEventsHandler> AppEventsHandler::onApplicationQuitHandler = nullptr;

void AppEventsHandler::Create(){
    if(onApplicationQuitHandler == nullptr)
    {
        onApplicationQuitHandler = std::make_unique<AppEventsHandler>();
    }
};