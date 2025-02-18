#pragma once
#include "../ECS/ECSManager.h"
#include <glm/glm.hpp>

struct CameraComponent : public EComponentS<CameraComponent>{
public:
    glm::vec2 zoom;
    int display;
    glm::vec2 size;

    CameraComponent(glm::vec2 zoom, int display, glm::vec2 size) : zoom(zoom), display(display), size(size){};
};