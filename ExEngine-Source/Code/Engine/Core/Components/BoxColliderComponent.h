#pragma once
#include "../ECS/ECSManager.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "../Utils/ExRect.h"
#include "../Utils/Algorithms/JsonExtensions.h"
#include <glm/glm.hpp>

struct BoxColliderComponent : public EComponentS<BoxColliderComponent>{
public:
    static constexpr unsigned int ComponentId = 3;

    ExRect exRect;

    BoxColliderComponent() = default;
    BoxColliderComponent(const BoxColliderComponent& boxCollider){
        exRect = boxCollider.exRect;
    };
    ~BoxColliderComponent() = default;

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), exRect, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"exRect", exRect.ToJson()}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("exRect")) exRect.FromJson(json["exRect"]);
    };
};

REGISTER_COMPONENT(BoxColliderComponent)