#pragma once
#include "../Rendering/Systems/RenderingSystem2D.h"
#include "../ECS/ECSManager.h"

class ClickSystem : public ECSystem{
public:
    ClickSystem();

    void UpdateSystem() override;

    inline const char* SystemName() override { return TYPE_NAME(ClickSystem); }; 
};