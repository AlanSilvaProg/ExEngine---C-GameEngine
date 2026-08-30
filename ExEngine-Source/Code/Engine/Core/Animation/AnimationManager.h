#pragma once
#include <unordered_set>

struct AnimationManager{
private:
    inline static std::unordered_set<int> entityToTriggerAnimation;
    inline static std::unordered_set<int> entityToStopAnimation;
    inline static std::unordered_set<int> entityWithPlayingAnimation;

public:
    inline static int ScheduleEntityToAnimate(const int id){ entityToTriggerAnimation.emplace(id); };
    inline static int ScheduleEntityToStopAnimation(const int id){ entityToStopAnimation.emplace(id); };
    inline static int InsertEntityPlayingAnimation(const int id){ entityWithPlayingAnimation.emplace(id); };

    inline static std::unordered_set<int>& GetEntitiesToTriggerAnimationQueue(){ return entityToTriggerAnimation; };
    inline static std::unordered_set<int>& GetEntitiesToStopAnimationQueue(){ return entityToStopAnimation; };
    inline static std::unordered_set<int>& GetEntitiesWithPlayingAnimationQueue(){ return entityWithPlayingAnimation; };
};