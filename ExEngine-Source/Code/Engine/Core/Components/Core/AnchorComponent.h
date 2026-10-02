#pragma once
#include <glm/glm.hpp>
#include "../../ECS/ECSManager.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"
#include "../../Utils/Algorithms/JsonExtensions.h"

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