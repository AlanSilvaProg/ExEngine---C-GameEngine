#pragma once
#include "../RenderingSystem2D/RenderingSystem2D.h"
#include "../UI/TextLabelSystem.h"
#include "../../ECS/ECSManager.h"
#include "../../Components/Rendering/CameraSafeAreaComponent.h"
#include <memory>

class CameraSystem : public ECSystem{
private:
    std::shared_ptr<RenderingSystem2D> renderingSystem;
    std::shared_ptr<TextLabelSystem> textLabelSystem;

    void RecalculateZoom(std::shared_ptr<CameraSafeAreaComponent>& cameraSafeAreaComponent);
public:
    CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem, std::shared_ptr<TextLabelSystem> textLabelSystem);

    void UpdateSystem(SystemContext systemContext) override;
    void RenderCamera(std::shared_ptr<EntityCS> camera, SystemContext systemContext);

    inline const char* SystemName() override { return TYPE_NAME(CameraSystem); };
};