#include "PhysicsEngineEventHandler.h"
#include <utility>

std::unique_ptr<PhysicsEngineEventHandler> PhysicsEngineEventHandler::entityWasClickedHandler = nullptr;

void PhysicsEngineEventHandler::Create(){
    if(entityWasClickedHandler == nullptr)
    {
        entityWasClickedHandler = std::make_unique<PhysicsEngineEventHandler>();
    }
};

void PhysicsEngineEventHandler::NotifyEntityWasClicked(unsigned int entityId) {
    if(entityWasClickedHandler != nullptr) {
        (*entityWasClickedHandler)(std::move(entityId));
    }
};
