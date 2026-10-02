#pragma once
#include "../../Engine/Core/Components/Core/TransformComponent.h"
#include "../../Engine/Core/ECSystems/RenderingSystem2D/RenderingSystem2D.h"
#include "../../Engine/Core/ECSystems/UI/TextLabelSystem.h"
#include <glm/glm.hpp>
#include <memory>

class EditorCameraController{
private:
    std::shared_ptr<TransformComponent> transform;
    std::shared_ptr<RenderingSystem2D> renderingSystem;
    std::shared_ptr<TextLabelSystem> textLabelSystem;

    glm::vec2 lastMousePos;

    bool overridingGlobalZoom = false;
    float savedGlobalZoom = 1.0f;

    void HandlePan();
    void HandleZoom();
    void FocusOnSelection();
public:
    EditorCameraController();

    void Update();
};
