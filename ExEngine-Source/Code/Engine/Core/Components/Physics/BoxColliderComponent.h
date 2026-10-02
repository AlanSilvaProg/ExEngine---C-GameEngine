#pragma once
#include "../../ECS/ECSManager.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"
#include "../../Utils/ExRect.h"
#include "../../Utils/Algorithms/JsonExtensions.h"
#include <glm/glm.hpp>

struct BoxColliderComponent : public EComponentS<BoxColliderComponent>{
public:
    static constexpr unsigned int ComponentId = 3;
    static constexpr const char* ComponentGroup = "Physics";

    ExRect exRect;
    bool showGizmo = true;

    BoxColliderComponent() = default;
    BoxColliderComponent(const BoxColliderComponent& boxCollider){
        exRect = boxCollider.exRect;
        showGizmo = boxCollider.showGizmo;
    };
    ~BoxColliderComponent() = default;

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), exRect, true),
        EX_SERIALIZER((*this), showGizmo, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"exRect", exRect.ToJson()},
            {"showGizmo", showGizmo}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("exRect")) exRect.FromJson(json["exRect"]);
        if (json.contains("showGizmo")) showGizmo = json["showGizmo"].get<bool>();
    };
};

REGISTER_COMPONENT(BoxColliderComponent)