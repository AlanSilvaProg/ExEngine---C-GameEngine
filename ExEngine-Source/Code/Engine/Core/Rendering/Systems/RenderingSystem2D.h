#pragma once
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"
#include "../../AssetManager/AssetManager.h"
#include "../../CameraSystem/CameraSystem.h"

class RenderingSystem2D : public ECSystem{
private: 
    std::shared_ptr<AssetManager> assetManager;
    std::shared_ptr<CameraSystem> cameraSystem;
    bool RenderOrderCheck(const EntityCS& a, const EntityCS& b);
public:
    RenderingSystem2D(std::shared_ptr<CameraSystem> camSystem);

    void UpdateSystem() override;
};