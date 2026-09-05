#pragma once
#include <unordered_set>
#include "../ECS/ECSManager.h"
#include "../EngineGetters.h"
#include "System/AnimationSystem.h"

struct AnimationManager{
private:
    static std::unordered_set<int> entityToTriggerAnimation;
    static std::unordered_set<int> entityToStopAnimation;
    static std::unordered_set<int> entityWithPlayingAnimation;

public:
    static void InitializeAnimationSystem();

    static int ScheduleEntityToAnimate(const int id);
    static int ScheduleEntityToStopAnimation(const int id);
    static int InsertEntityPlayingAnimation(const int id);

    static std::unordered_set<int>& GetEntitiesToTriggerAnimationQueue();
    static std::unordered_set<int>& GetEntitiesToStopAnimationQueue();
    static std::unordered_set<int>& GetEntitiesWithPlayingAnimationQueue();
};