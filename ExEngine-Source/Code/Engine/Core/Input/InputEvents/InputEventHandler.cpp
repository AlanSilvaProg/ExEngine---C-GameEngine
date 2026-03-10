#include "InputEventHandler.h"
#include <memory>

std::unique_ptr<InputEventHandler> InputEventHandler::handler = nullptr;

void InputEventHandler::Create(){
    if(handler == nullptr)
    {
        handler = std::make_unique<InputEventHandler>();
    }
};