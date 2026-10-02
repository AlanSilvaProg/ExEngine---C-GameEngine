#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "../../ECS/ECSManager.h"
#include "../../Components/Core/AnchorComponent.h"
#include "../../Components/Core/TransformComponent.h"
#include "../Transform/TransformUtils.h"

namespace AnchorUtils{
    // The world-space point everything that's drawn/tested relative to this entity's transform
    // (sprite, box collider, ...) should actually be anchored around: with no AnchorComponent it's
    // just the transform's own world position, composed through every parent (unchanged behavior
    // for a root entity); with an AnchorComponent, it's shifted back by the anchor's local offset
    // so that offset acts as the entity's pivot instead of its raw position.
    inline glm::vec2 GetPivotAdjustedPosition(const std::shared_ptr<EntityCS>& entity, const std::shared_ptr<TransformComponent>& transformComponent){
        glm::vec2 position = TransformUtils::GetWorldTransform(entity).position;
        if(auto anchor = entity->GetComponent<AnchorComponent>())
            position -= glm::vec2(anchor->position.x, anchor->position.y);
        return position;
    }

};