#pragma once
#include "../ECS/ECSManager.h"

struct CameraComponent : public EComponentS<CameraComponent>{
public:
    int display;

    CameraComponent(int display) : display(display){};
};