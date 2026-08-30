#pragma once
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"
#include "../../AssetManager/AssetManager.h"

class RenderingSystem2D : public ECSystem{
private: 
    std::shared_ptr<AssetManager> assetManager;
    bool RenderOrderCheck(const std::shared_ptr<EntityCS> a, const std::shared_ptr<EntityCS> b);
public:
    RenderingSystem2D();

    void UpdateSystem(SystemContext systemContext) override;
    
    inline const char* SystemName() override { return TYPE_NAME(RenderingSystem2D); }; 
};