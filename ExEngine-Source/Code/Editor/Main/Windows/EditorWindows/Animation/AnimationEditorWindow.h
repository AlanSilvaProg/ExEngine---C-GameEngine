#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Animation/Component/AnimationComponent.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include "nlohmann/json.hpp"
#include <functional>
#include <memory>

class AnimationEditorWindow : public EditorWindow{
private:
    std::shared_ptr<EntityCS> currentEntity;
    nlohmann::json currentEntityOriginalState;

    float timelinePixelsPerSecond = 100.0f;
    float previewPlayheadTime = 0.0f; // playhead position while no AnimationComponent is bound
    int currentAnimation;
    bool isPlaying = false;
    float selectedKeyframeTime = -1.0f; // negative means no keyframe is selected

    bool isDraggingKeyframe = false;
    float draggingKeyframeTime = -1.0f; // tracks the dragged keyframe's current time as it moves

    bool showUnsavedChangesPopup = false; // a confirmation flow is in progress
    bool requestOpenUnsavedChangesPopup = false; // one-shot: call ImGui::OpenPopup on the next frame
    std::function<void()> pendingConfirmedAction; // runs only if Save or Discard is chosen, never on Cancel

    // True whenever a keyframe was added/removed/moved or an animation was renamed since the
    // last save/discard. Gates whether switching entity/animation needs confirmation at all.
    bool hasUnsavedChanges = false;

    void DrawEntityInfo(const std::shared_ptr<EntityCS> entity, const std::shared_ptr<AnimationComponent> animationComponent);
    void DrawTimeline(const std::shared_ptr<EntityCS> entity, const std::shared_ptr<AnimationComponent> animationComponent);

    void AddKeyframe(const std::shared_ptr<EntityCS> entity);
    void SaveChanges(const std::shared_ptr<EntityCS> entity);
    void DeleteSelectedKeyframe();
    void SetSelectedKeyframeTime(const float time);
    void CacheEntityState();
    void ResetEntityState();
    void SwitchToEntity(const std::shared_ptr<EntityCS> entity);
    void SwitchToAnimation(const int newAnimationIndex);
    void RequestConfirmedAction(std::function<void()> action);
    void PerformOrConfirm(std::function<void()> action); // shows the confirmation popup only if hasUnsavedChanges
    void ApplyEvaluatedState(const std::shared_ptr<EntityCS> entity, AnimationStep* currentAnimationState);
public:
    AnimationEditorWindow();

    void Draw(const int phase) override; //0 == early 1 == late
};
