#pragma once
#include "../../Engine/Core/EventSystem/EventNotifier.h"
#include <memory>

class EditorCommandEventHandler : public EventNotifier {
public:
    static std::unique_ptr<EditorCommandEventHandler> duplicate;
    static std::unique_ptr<EditorCommandEventHandler> deleteCmmd;
    static std::unique_ptr<EditorCommandEventHandler> focusSelected;

    static void Create();
};