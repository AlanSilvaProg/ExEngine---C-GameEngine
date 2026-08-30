#pragma once
#include "../Component/AnimationComponent.h"
#include "../../ECS/ECSManager.h"
#include <queue>
#include <memory>

class AnimationSystem : public ECSystem{
private:
    std::unordered_set<int>& entitiesToTrigger;
    std::unordered_set<int>& entitiesToStop;
    std::unordered_set<int>& entitiesPlaying;

    void StartEntityAnimation(const int entityId, const std::shared_ptr<AnimationComponent>& animationComponent);
    void StopEntityAnimation(const int entityId, const std::shared_ptr<AnimationComponent>& animationComponent);
public:
    AnimationSystem();

    void UpdateSystem(SystemContext systemContext) override;

    inline const char* SystemName() override { return TYPE_NAME(AnimationSystem); };
};