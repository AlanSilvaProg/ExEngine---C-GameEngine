#pragma once
#include <glm/glm.hpp>
#include "../ECS/ECSManager.h"
#include "../Serializer/ISerializable.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "../Utils/Algorithms/JsonExtensions.h"

struct TransformComponent : public EComponentS<TransformComponent>{
public:
    static constexpr unsigned int ComponentId = 1;

    glm::vec3 position = glm::vec3(0,0,0);
    glm::vec3 rotation = glm::vec3(0,0,0);
    glm::vec3 scale = glm::vec3(1,1,1);

    TransformComponent() = default;
    TransformComponent(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale){};
    TransformComponent(const TransformComponent& transform){
        position = transform.position;
        rotation = transform.rotation;
        scale = transform.scale;
    };

    inline void Move(const glm::vec3 position){
        this->position += position;
    };

    inline void Scale(const glm::vec3 scale){
        this->scale += scale;
    };

    inline void Rotate(const glm::vec3 rotation){
        this->rotation += rotation;
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), position, true),
        EX_SERIALIZER((*this), rotation, true),
        EX_SERIALIZER((*this), scale, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"position", JsonExtensions::glm_to_json(position)},
            {"rotation", JsonExtensions::glm_to_json(rotation)},
            {"scale", JsonExtensions::glm_to_json(scale)}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("position")) JsonExtensions::glm_from_json(json["position"], position);
        if (json.contains("rotation")) JsonExtensions::glm_from_json(json["rotation"], rotation);
        if (json.contains("scale")) JsonExtensions::glm_from_json(json["scale"], scale);
    }
};

REGISTER_COMPONENT(TransformComponent)