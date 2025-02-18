#pragma once
#include "../ECS/ECSManager.h"

class CameraSystem : public ECSystem{
public:
    CameraSystem();

    void UpdateSystem() override;
};