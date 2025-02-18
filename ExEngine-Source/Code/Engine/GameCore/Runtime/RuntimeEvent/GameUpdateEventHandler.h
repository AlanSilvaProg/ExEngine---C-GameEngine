#pragma once
#include <memory>
#include "../../../Core/EventSystem/Event.h"

class GameUpdateEventHandler : public Event<int>{
public:
    static std::unique_ptr<GameUpdateEventHandler> earlyhandler;
    static std::unique_ptr<GameUpdateEventHandler> latehandler;

    static void Create();
};