#include "ClickSystem.h"
#include "ExPhysicsEngine.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../Input/Input.h"
#include <algorithm>

ClickSystem::ClickSystem(std::shared_ptr<ECSManager> ecsManager) : ecsManager(ecsManager){
    Require<BoxColliderComponent>(false);
    Require<TransformComponent>(false);
};

void ClickSystem::UpdateSystem(){
    if(Input::GetMouseButtonDown(SDL_BUTTON_LEFT)){
        auto cameraTransformComponent = ExRendererGetters::currentRenderCameraTransform;
        if(cameraTransformComponent == nullptr)
            return;

        auto entities = systemEntities;

        std::erase_if(entities, [cameraTransformComponent](std::shared_ptr<EntityCS> entity){
            return cameraTransformComponent->position.z > entity->GetComponent<TransformComponent>()->position.z;
        });
        
        std::sort(entities.begin(), entities.end(), [](std::shared_ptr<EntityCS> firstEntity, std::shared_ptr<EntityCS> secondEntity){
            return firstEntity->GetComponent<TransformComponent>()->position.z 
            <= secondEntity->GetComponent<TransformComponent>()->position.z;
        });

        auto mousePosition = Input::GetMousePosition();
        glm::vec2 worldMousePosition = glm::vec2(mousePosition.x, mousePosition.y) + glm::vec2(cameraTransformComponent->position.x, cameraTransformComponent->position.y);
        
        for(auto entity : entities){
            if(CheckBounds(entity, worldMousePosition))
            {
                ExPhysicsEngine::EntityWasClicked(entity->GetId());
            }
        }
    }
};

bool ClickSystem::CheckBounds(const std::shared_ptr<EntityCS> entity, const glm::vec2& worldMousePosition) const{
    auto transformComponent = entity->GetComponent<TransformComponent>();
    auto boxColliderComponent = entity->GetComponent<BoxColliderComponent>();
    glm::vec2 entityPosition(transformComponent->position.x, transformComponent->position.y);

    glm::vec2 rectMin = entityPosition + glm::min(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);
    glm::vec2 rectMax = entityPosition + glm::max(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);

    bool clickedInsideRect = worldMousePosition.x >= rectMin.x && worldMousePosition.x <= rectMax.x
        && worldMousePosition.y >= rectMin.y && worldMousePosition.y <= rectMax.y;

    return clickedInsideRect;
};