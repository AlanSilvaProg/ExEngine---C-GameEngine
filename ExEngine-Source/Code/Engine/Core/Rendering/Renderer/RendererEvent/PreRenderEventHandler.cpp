#include "PreRenderEventHandler.h"

std::unique_ptr<PreRenderEventHandler> PreRenderEventHandler::handler = nullptr;

void PreRenderEventHandler::Create(){
    if(handler == nullptr)
    {
        handler = std::make_unique<PreRenderEventHandler>();
    }
};