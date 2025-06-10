#pragma once
#include <glm/glm.hpp>
#include "../ECS/ECSManager.h"

struct TransformComponent : public EComponentS<TransformComponent>{
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    TransformComponent(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position(position), rotation(rotation), scale(scale){};

    void Move(const glm::vec3 movement){
        position += movement;
    };
};