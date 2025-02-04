#pragma once
#include <memory>
#include "../../../Core/EventSystem/Event.h"

class GameEarlyUpdateEventHandler : public Event<int>{
public:
    static std::unique_ptr<GameEarlyUpdateEventHandler> handler;

    static void Create();
};