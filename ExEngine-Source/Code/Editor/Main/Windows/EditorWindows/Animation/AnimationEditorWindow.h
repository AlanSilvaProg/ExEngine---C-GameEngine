#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Animation/Component/AnimationComponent.h"
#include <memory>

class AnimationEditorWindow : public EditorWindow{
private:
    float timelinePixelsPerSecond = 100.0f;
    float previewPlayheadTime = 0.0f; // playhead position while no AnimationComponent is bound

    void DrawEntityInfo(const std::shared_ptr<EntityCS> entity);
    void DrawTimeline(const std::shared_ptr<AnimationComponent> animationComponent);

    void AddKeyframe();
    void SaveData();
    void ResetEntityState();
public:
    void Draw(const int phase) override; //0 == early 1 == late
};
