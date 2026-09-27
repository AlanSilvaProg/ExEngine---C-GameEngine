#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"
#include "../../../Engine/Core/Components/AnchorComponent.h"
#include "../../../Engine/Core/Components/TransformComponent.h"
#include <glm/glm.hpp>

class EditorDrawAnchorSystem : public ECSystem{
private:
    bool isDragging = false;
    unsigned int draggedEntityId = 0;
    // World position of the sprite's top-left corner, captured when the drag starts, so the
    // anchor can be re-derived from it each frame and the sprite never visibly moves.
    glm::vec2 dragStartSpriteTopLeft = glm::vec2(0.0f, 0.0f);

public:
    EditorDrawAnchorSystem();

    void UpdateSystem(SystemContext systemContext) override;

    inline const char* SystemName() override { return TYPE_NAME(EditorDrawAnchorSystem); };
};
