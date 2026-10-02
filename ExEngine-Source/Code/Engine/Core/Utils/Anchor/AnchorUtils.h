#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "../../ECS/ECSManager.h"
#include "../../Components/Core/AnchorComponent.h"
#include "../../Components/Core/TransformComponent.h"

namespace AnchorUtils{
    // The world-space point everything that's drawn/tested relative to this entity's transform
    // (sprite, box collider, ...) should actually be anchored around: with no AnchorComponent it's
    // just the transform's own position (unchanged behavior); with one, it's shifted back by the
    // anchor's local offset so that offset acts as the entity's pivot instead of its raw position.
    inline glm::vec2 GetPivotAdjustedPosition(const std::shared_ptr<EntityCS>& entity, const std::shared_ptr<TransformComponent>& transformComponent){
        glm::vec2 position(transformComponent->position.x, transformComponent->position.y);
        if(auto anchor = entity->GetComponent<AnchorComponent>())
            position -= glm::vec2(anchor->position.x, anchor->position.y);
        return position;
    }

};