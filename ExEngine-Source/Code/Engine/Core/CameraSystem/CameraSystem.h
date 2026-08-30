#pragma once
#include "../Rendering/Systems/RenderingSystem2D.h"
#include "../ECS/ECSManager.h"
#include <memory>

class CameraSystem : public ECSystem{
private:
    std::shared_ptr<RenderingSystem2D> renderingSystem;

public:
    CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem);

    void UpdateSystem(SystemContext systemContext) override;
    void RenderCamera(std::shared_ptr<EntityCS> camera, SystemContext systemContext);

    inline const char* SystemName() override { return TYPE_NAME(CameraSystem); };
};