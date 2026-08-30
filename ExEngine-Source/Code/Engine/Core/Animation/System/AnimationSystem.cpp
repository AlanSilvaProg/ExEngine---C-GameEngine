#include "AnimationSystem.h"
#include "../AnimationManager.h"

AnimationSystem::AnimationSystem() : entitiesToTrigger(AnimationManager::GetEntitiesToTriggerAnimationQueue()),
      entitiesToStop(AnimationManager::GetEntitiesToStopAnimationQueue()),
      entitiesPlaying(AnimationManager::GetEntitiesWithPlayingAnimationQueue())
{
    Require<AnimationComponent>(false);
};

void AnimationSystem::UpdateSystem(SystemContext systemContext){
    for(const auto& entity : systemEntities)
    {
        auto entityAnimation = entity->GetComponent<AnimationComponent>();
        const auto entityId = entity->GetId();

        if(entitiesToStop.contains(entityId)){
            StopEntityAnimation(entityId, entityAnimation);
            continue;
        }
        
        if(entitiesToTrigger.contains(entityId)){
            StartEntityAnimation(entityId, entityAnimation);
            continue;
        }

        if(entitiesPlaying.contains(entityId)){
            const auto currentAnimationState = entityAnimation->Evaluate(systemContext);

            if(currentAnimationState == nullptr || currentAnimationState->WasReturned()) continue;

            for(const auto& componentUpdate : currentAnimationState->GetStepStateContent().componentUpdates)
            {
                entity->ApplyComponentUpdate(componentUpdate);
            }

            if(!entityAnimation->IsRunning()) entitiesPlaying.erase(entityId);
        }
    }
};

void AnimationSystem::StartEntityAnimation(const int entityId, const std::shared_ptr<AnimationComponent>& animationComponent){
    animationComponent->Start();
    entitiesToTrigger.erase(entityId);
    AnimationManager::InsertEntityPlayingAnimation(entityId);
};

void AnimationSystem::StopEntityAnimation(const int entityId, const std::shared_ptr<AnimationComponent>& animationComponent){
    animationComponent->Stop();
    entitiesPlaying.erase(entityId);
    entitiesToStop.erase(entityId);
};