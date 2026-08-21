#include "ClickSystem.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../Input/Input.h"

ClickSystem::ClickSystem(){
    Require<BoxColliderComponent>(false);
    Require<TransformComponent>(false);
};

void ClickSystem::UpdateSystem(){
    const auto& entities = systemEntities;
    
    if(Input::GetMouseButtonDown(0)){
        auto mousePosition = Input::GetMousePosition();

        for(auto entity : entities){
            auto transformComponent = entity->GetComponent<TransformComponent>();
            auto boxColliderComponent = entity->GetComponent<BoxColliderComponent>();
        }
    }
};