#include "PreRenderEventHandler.h"

std::unique_ptr<PreRenderEventHandler> PreRenderEventHandler::preRenderHandler= nullptr;
std::unique_ptr<PreRenderEventHandler> PreRenderEventHandler::postRenderHandler = nullptr;
std::unique_ptr<PreRenderEventHandler> PreRenderEventHandler::postRenderPresentHandler = nullptr;

void PreRenderEventHandler::Create(){
    if(preRenderHandler == nullptr)
    {
        preRenderHandler = std::make_unique<PreRenderEventHandler>();
    }

    if(postRenderHandler == nullptr)
    {
        postRenderHandler = std::make_unique<PreRenderEventHandler>();
    }

    if(postRenderPresentHandler == nullptr)
    {
        postRenderPresentHandler = std::make_unique<PreRenderEventHandler>();
    }
};