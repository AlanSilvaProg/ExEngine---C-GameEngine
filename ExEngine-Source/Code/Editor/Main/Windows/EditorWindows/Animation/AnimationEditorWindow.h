#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Animation/Component/AnimationComponent.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include "nlohmann/json.hpp"
#include <memory>

class AnimationEditorWindow : public EditorWindow{
private:
    std::shared_ptr<EntityCS> currentEntity;
    nlohmann::json currentEntityOriginalState;

    float timelinePixelsPerSecond = 100.0f;
    float previewPlayheadTime = 0.0f; // playhead position while no AnimationComponent is bound
    bool isPlaying = false;
    float selectedKeyframeTime = -1.0f; // negative means no keyframe is selected

    bool isDraggingKeyframe = false;
    float draggingKeyframeTime = -1.0f; // tracks the dragged keyframe's current time as it moves

    void DrawEntityInfo(const std::shared_ptr<EntityCS> entity);
    void DrawTimeline(const std::shared_ptr<EntityCS> entity, const std::shared_ptr<AnimationComponent> animationComponent);

    void AddKeyframe(const std::shared_ptr<EntityCS> entity);
    void SaveChanges(const std::shared_ptr<EntityCS> entity);
    void DeleteSelectedKeyframe();
    void SetSelectedKeyframeTime(const float time);
    void CacheEntityState();
    void ResetEntityState();
    void ApplyEvaluatedState(const std::shared_ptr<EntityCS> entity, AnimationStep* currentAnimationState);
public:
    AnimationEditorWindow();

    void Draw(const int phase) override; //0 == early 1 == late
};
