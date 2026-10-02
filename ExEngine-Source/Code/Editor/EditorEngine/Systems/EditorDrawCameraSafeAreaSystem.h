#pragma once
#include "../../../Engine/Core/ECS/ECSManager.h"
#include "../../../Engine/Core/Components/Rendering/CameraSafeAreaComponent.h"
#include <glm/glm.hpp>

enum class SafeAreaHandle{
    PositionBegin,
    PositionEnd,
    LimitBegin,
    LimitEnd
};

class EditorDrawCameraSafeAreaSystem : public ECSystem{
private:
    bool isDragging = false;
    unsigned int draggedEntityId = 0;
    SafeAreaHandle draggedHandle = SafeAreaHandle::PositionBegin;

public:
    EditorDrawCameraSafeAreaSystem();

    void UpdateSystem(SystemContext systemContext) override;

    inline const char* SystemName() override { return TYPE_NAME(EditorDrawCameraSafeAreaSystem); };
};
