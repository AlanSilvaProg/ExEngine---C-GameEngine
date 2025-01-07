#pragma once
#include "../../ECS/ECSManager.h"

class RenderingSystem2D : public ECSystem{
public:

    //Start To render stuffs

    void UpdateSystem() override;
    void SetupRequirements() override;
};