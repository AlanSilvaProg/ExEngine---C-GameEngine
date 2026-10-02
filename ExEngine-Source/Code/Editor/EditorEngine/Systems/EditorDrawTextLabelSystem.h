#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"
#include "../../../Engine/Core/Components/UI/TextLabelComponent.h"
#include "../../../Engine/Core/Components/Core/TransformComponent.h"
#include <glm/glm.hpp>

class EditorDrawTextLabelSystem : public ECSystem{
private:
    bool isDragging = false;
    unsigned int draggedEntityId = 0;
    glm::vec2 fixedLocalCorner = glm::vec2(0.0f, 0.0f);

public:
    EditorDrawTextLabelSystem();

    void UpdateSystem(SystemContext systemContext) override;

    inline const char* SystemName() override { return TYPE_NAME(EditorDrawTextLabelSystem); };
};
