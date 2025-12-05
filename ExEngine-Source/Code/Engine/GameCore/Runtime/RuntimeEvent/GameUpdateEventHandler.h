#pragma once
#include <memory>
#include "../../../Core/EventSystem/EventNotifier.h"

class GameUpdateEventHandler : public EventNotifier{
public:
    static std::unique_ptr<GameUpdateEventHandler> earlyHandler;
    static std::unique_ptr<GameUpdateEventHandler> updateHandler;
    static std::unique_ptr<GameUpdateEventHandler> fixedUpdateHandler;
    static std::unique_ptr<GameUpdateEventHandler> lateHandler;

    static void Create();
};