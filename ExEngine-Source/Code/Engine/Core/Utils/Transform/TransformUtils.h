#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include <memory>
#include "../../ECS/ECSManager.h"
#include "../../Components/Core/TransformComponent.h"

namespace TransformUtils{

    struct WorldTransform{
        glm::vec2 position = glm::vec2(0.0f, 0.0f);
        float rotation = 0.0f; // degrees
        glm::vec2 scale = glm::vec2(1.0f, 1.0f);
    };

    // Composes this entity's local TransformComponent with every ancestor's, scale -> rotation ->
    // position (same order a Unity/Godot-style 2D hierarchy uses), so moving/rotating/scaling a
    // parent carries its descendants along with it. Returns identity-ish defaults for an entity
    // with no TransformComponent, and just the local transform for one with no parent.
    inline WorldTransform GetWorldTransform(const std::shared_ptr<EntityCS>& entity){
        if(entity == nullptr) return WorldTransform{};

        auto transformComponent = entity->GetComponent<TransformComponent>();
        const glm::vec2 localPosition = transformComponent != nullptr ? glm::vec2(transformComponent->position.x, transformComponent->position.y) : glm::vec2(0.0f, 0.0f);
        const float localRotation = transformComponent != nullptr ? transformComponent->rotation.x : 0.0f;
        const glm::vec2 localScale = transformComponent != nullptr ? glm::vec2(transformComponent->scale.x, transformComponent->scale.y) : glm::vec2(1.0f, 1.0f);

        auto parent = entity->GetParent();
        if(parent == nullptr) return WorldTransform{ localPosition, localRotation, localScale };

        const WorldTransform parentWorld = GetWorldTransform(parent);

        const float radians = glm::radians(parentWorld.rotation);
        const float cosR = std::cos(radians);
        const float sinR = std::sin(radians);

        const glm::vec2 scaledLocal = localPosition * parentWorld.scale;
        const glm::vec2 rotatedLocal(
            scaledLocal.x * cosR - scaledLocal.y * sinR,
            scaledLocal.x * sinR + scaledLocal.y * cosR
        );

        return WorldTransform{
            parentWorld.position + rotatedLocal,
            parentWorld.rotation + localRotation,
            parentWorld.scale * localScale
        };
    };

    // Inverse of composing a local position under `parent`'s world transform - used to convert a
    // freshly-read world-space position (e.g. the mouse, already in world space) back into the
    // local space a dragged entity's TransformComponent::position should be written in.
    inline glm::vec2 WorldToLocal(const glm::vec2& worldPosition, const std::shared_ptr<EntityCS>& parent){
        if(parent == nullptr) return worldPosition;

        const WorldTransform parentWorld = GetWorldTransform(parent);

        const glm::vec2 delta = worldPosition - parentWorld.position;

        const float radians = glm::radians(-parentWorld.rotation);
        const float cosR = std::cos(radians);
        const float sinR = std::sin(radians);

        const glm::vec2 unrotated(
            delta.x * cosR - delta.y * sinR,
            delta.x * sinR + delta.y * cosR
        );

        constexpr float MIN_SCALE = 0.0001f;
        const glm::vec2 safeScale(
            std::abs(parentWorld.scale.x) > MIN_SCALE ? parentWorld.scale.x : MIN_SCALE,
            std::abs(parentWorld.scale.y) > MIN_SCALE ? parentWorld.scale.y : MIN_SCALE
        );

        return unrotated / safeScale;
    };

};
