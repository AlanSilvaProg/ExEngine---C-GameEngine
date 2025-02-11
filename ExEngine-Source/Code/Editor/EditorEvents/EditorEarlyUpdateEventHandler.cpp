#include "EditorEarlyUpdateEventHandler.h"

std::unique_ptr<EditorEarlyUpdateEventHandler> EditorEarlyUpdateEventHandler::handler = nullptr;

void EditorEarlyUpdateEventHandler::Create(){
    if(handler == nullptr)
    {
        handler = std::make_unique<EditorEarlyUpdateEventHandler>();
    }
};