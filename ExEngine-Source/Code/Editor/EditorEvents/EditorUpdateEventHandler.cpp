#include "EditorUpdateEventHandler.h"

std::unique_ptr<EditorUpdateEventHandler> EditorUpdateEventHandler::earlyHandler = nullptr;
std::unique_ptr<EditorUpdateEventHandler> EditorUpdateEventHandler::lateHandler = nullptr;

void EditorUpdateEventHandler::Create(){
    if(earlyHandler == nullptr)
    {
        earlyHandler = std::make_unique<EditorUpdateEventHandler>();
    }

    if(lateHandler == nullptr)
    {
        lateHandler = std::make_unique<EditorUpdateEventHandler>();
    }
};