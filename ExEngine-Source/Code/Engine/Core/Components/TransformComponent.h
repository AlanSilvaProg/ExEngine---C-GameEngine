#pragma once
#include <glm/glm.hpp>
#include "../ECS/ECSManager.h"
#include "../Serializer/ISerializable.h"

struct TransformComponent : public EComponentS<TransformComponent>{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    TransformComponent(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale){};

    void Move(const glm::vec3 movement){
        position += movement;
    };

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER((*this), position, true),
                EX_SERIALIZER((*this), rotation, true),
                EX_SERIALIZER((*this), scale, true)
            }
        };
    };
};