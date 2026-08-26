#include "EditorCommandEventHandler.h"

std::unique_ptr<EditorCommandEventHandler> EditorCommandEventHandler::duplicate = nullptr;
std::unique_ptr<EditorCommandEventHandler> EditorCommandEventHandler::deleteCmmd = nullptr;
std::unique_ptr<EditorCommandEventHandler> EditorCommandEventHandler::focusSelected = nullptr;

void EditorCommandEventHandler::Create(){
    if(duplicate == nullptr)
    {
        duplicate = std::make_unique<EditorCommandEventHandler>();
    }
    if(deleteCmmd == nullptr)
    {
        deleteCmmd = std::make_unique<EditorCommandEventHandler>();
    }
    if(focusSelected == nullptr)
    {
        focusSelected = std::make_unique<EditorCommandEventHandler>();
    }
};