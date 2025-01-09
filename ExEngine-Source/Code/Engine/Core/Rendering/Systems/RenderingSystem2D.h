#pragma once
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"

class RenderingSystem2D : public ECSystem{
private:
    bool RenderOrderCheck(const EntityCS& a, const EntityCS& b);
public:
    RenderingSystem2D();

    void UpdateSystem() override;
};