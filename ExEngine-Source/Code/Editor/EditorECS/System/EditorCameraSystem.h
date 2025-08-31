#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"
#include "../../../Engine/Core/Rendering/Systems/RenderingSystem2D.h"
#include <memory>

class EditorCameraSystem : public ECSystem{
private: 
    std::shared_ptr<RenderingSystem2D> renderingSystem;

    void UpdateDisplayTexture();
    void DisableDisplayTextures();
public:
    EditorCameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem);

    void UpdateSystem() override;
};