#pragma once
#include "../../Engine/Core/EventSystem/EventNotifier.h"
#include <memory>

class EditorEarlyUpdateEventHandler : public EventNotifier {
public:
    static std::unique_ptr<EditorEarlyUpdateEventHandler> handler;

    static void Create();
};