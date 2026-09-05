#include "AnimationManager.h"
#include "../EngineGetters.h"

std::unordered_set<int> AnimationManager::entityToTriggerAnimation;
std::unordered_set<int> AnimationManager::entityToStopAnimation;
std::unordered_set<int> AnimationManager::entityWithPlayingAnimation;

void AnimationManager::InitializeAnimationSystem(){
    const auto animationSystem = EngineGetters::GetEnginePtr()->GetECSManagerPtr()->CreateSystem<AnimationSystem>();
    const auto context = EngineGetters::GetEnginePtr()->GetECSManagerPtr()->GetECSystemContext(SystemContext::UPDATE);
    context->Register(typeid(AnimationSystem), animationSystem);
};

int AnimationManager::ScheduleEntityToAnimate(const int id){
    entityToTriggerAnimation.emplace(id);
};

int AnimationManager::ScheduleEntityToStopAnimation(const int id){
    entityToStopAnimation.emplace(id);
};

int AnimationManager::InsertEntityPlayingAnimation(const int id){
    entityWithPlayingAnimation.emplace(id);
};

std::unordered_set<int>& AnimationManager::GetEntitiesToTriggerAnimationQueue(){
    return entityToTriggerAnimation;
};

std::unordered_set<int>& AnimationManager::GetEntitiesToStopAnimationQueue(){
    return entityToStopAnimation;
};

std::unordered_set<int>& AnimationManager::GetEntitiesWithPlayingAnimationQueue(){
    return entityWithPlayingAnimation;
};