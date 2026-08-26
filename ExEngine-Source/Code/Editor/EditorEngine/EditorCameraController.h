#pragma once
#include "../../Engine/Core/Components/TransformComponent.h"
#include "../../Engine/Core/Rendering/Systems/RenderingSystem2D.h"
#include <glm/glm.hpp>
#include <memory>

class EditorCameraController{
private:
    std::shared_ptr<TransformComponent> transform;
    std::shared_ptr<RenderingSystem2D> renderingSystem;

    glm::vec2 lastMousePos;

    void HandlePan();
    void FocusOnSelection();
public:
    EditorCameraController();

    void Update();
};
