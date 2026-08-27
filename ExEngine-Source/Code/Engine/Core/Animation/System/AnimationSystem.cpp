#include "AnimationSystem.h"
#include "../Component/AnimationComponent.h"

AnimationSystem::AnimationSystem(){
    Require<AnimationComponent>(false);
};

void AnimationSystem::UpdateSystem(){
    for(const auto& entity : systemEntities)
    {
        auto entityAnimation = entity->GetComponent<AnimationComponent>();

        entityAnimation->Evaluate();
    }
};