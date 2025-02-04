#pragma once
#include <memory>
#include "../../../Core/EventSystem/Event.h"

class GameLateUpdateEventHandler : public Event<int>{
public:
    static std::unique_ptr<GameLateUpdateEventHandler> handler;

    static void Create();
};