#pragma once
#include "../../ECS/ECSManager.h"

class AnimationSystem : public ECSystem{
public:
    AnimationSystem();

    void UpdateSystem() override;

    inline const char* SystemName() override { return TYPE_NAME(AnimationSystem); };
};