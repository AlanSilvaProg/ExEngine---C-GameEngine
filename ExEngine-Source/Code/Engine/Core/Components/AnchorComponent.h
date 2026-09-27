#pragma once
#include <glm/glm.hpp>
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "../Utils/Algorithms/JsonExtensions.h"
#include "TransformComponent.h"

struct AnchorComponent : public EComponentS<AnchorComponent>{
public:
    static constexpr unsigned int ComponentId = 5;
    static constexpr const char* ComponentGroup = "Core";

    glm::vec3 position = glm::vec3(0,0,0);

    AnchorComponent() = default;
    AnchorComponent(glm::vec3 position) : position(position){};
    AnchorComponent(const AnchorComponent& anchor){
        position = anchor.position;
    };

    // The world-space point everything that's drawn/tested relative to this entity's transform
    // (sprite, box collider, ...) should actually be anchored around: with no AnchorComponent it's
    // just the transform's own position (unchanged behavior); with one, it's shifted back by the
    // anchor's local offset so that offset acts as the entity's pivot instead of its raw position.
    static glm::vec2 GetPivotAdjustedPosition(const std::shared_ptr<EntityCS>& entity, const std::shared_ptr<TransformComponent>& transformComponent){
        glm::vec2 position(transformComponent->position.x, transformComponent->position.y);
        if(auto anchor = entity->GetComponent<AnchorComponent>())
            position -= glm::vec2(anchor->position.x, anchor->position.y);
        return position;
    }

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), position, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"position", JsonExtensions::glm_to_json(position)}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("position")) JsonExtensions::glm_from_json(json["position"], position);
    }
};

REGISTER_COMPONENT(AnchorComponent)