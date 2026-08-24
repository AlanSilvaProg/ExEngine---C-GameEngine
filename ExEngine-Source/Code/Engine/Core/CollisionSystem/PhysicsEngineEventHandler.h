#pragma once
#include <memory>
#include "../EventSystem/Event.h"

class PhysicsEngineEventHandler : public Event<unsigned int>{
public:
    static std::unique_ptr<PhysicsEngineEventHandler> entityWasClickedHandler;

    static void Create();
    static void NotifyEntityWasClicked(unsigned int entityId);
};
