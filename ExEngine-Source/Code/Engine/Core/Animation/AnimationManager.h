#pragma once
#include <unordered_set>
#include "../ECS/ECSManager.h"
#include "../EngineGetters.h"
#include "../ECSystems/AnimationSystem/AnimationSystem.h"

struct AnimationManager{
private:
    static std::unordered_set<int> entityToTriggerAnimation;
    static std::unordered_set<int> entityToStopAnimation;
    static std::unordered_set<int> entityWithPlayingAnimation;

public:
    static void InitializeAnimationSystem();

    static void ScheduleEntityToAnimate(const int id);
    static void ScheduleEntityToStopAnimation(const int id);
    static void InsertEntityPlayingAnimation(const int id);

    static std::unordered_set<int>& GetEntitiesToTriggerAnimationQueue();
    static std::unordered_set<int>& GetEntitiesToStopAnimationQueue();
    static std::unordered_set<int>& GetEntitiesWithPlayingAnimationQueue();
};