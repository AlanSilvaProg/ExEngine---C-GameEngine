#include "ResolutionChangeEventHandler.h"
#include <utility>

std::unique_ptr<ResolutionChangeEventHandler> ResolutionChangeEventHandler::resolutionChangeHandler = nullptr;

void ResolutionChangeEventHandler::Create(){
    if(resolutionChangeHandler == nullptr)
    {
        resolutionChangeHandler = std::make_unique<ResolutionChangeEventHandler>();
    }
};

void ResolutionChangeEventHandler::NotifyResolutionChange(int width, int height) {
    if(resolutionChangeHandler != nullptr) {
        (*resolutionChangeHandler)(std::move(width), std::move(height));
    }
};