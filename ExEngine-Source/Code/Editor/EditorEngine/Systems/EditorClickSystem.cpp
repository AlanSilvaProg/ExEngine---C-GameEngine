#include "EditorClickSystem.h"
#include "../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../Engine/Core/Input/Input.h"
#include "../../Main/EditorInterfaceGetters.h"
#include "../../Main/Windows/EditorWindows/ElementSelectionController.h"
#include <imgui.h>
#include <SDL.h>

EditorClickSystem::EditorClickSystem(){
    Require<BoxColliderComponent>(false);
    Require<TransformComponent>(false);
};

void EditorClickSystem::UpdateSystem(){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;
    if(ImGui::GetIO().WantCaptureMouse) return;

    if(Input::GetMouseButtonDown(SDL_BUTTON_LEFT))
    {
        mouseDownTime = ImGui::GetTime();
    }

    if(!Input::GetMouseButtonUp(SDL_BUTTON_LEFT)) return;

    // Between down and up there's a minimum time to be considered a hold/drag (and be ignored),
    // reusing the same "system default" timing the Asset Browser uses for its own click gestures.
    double heldTime = ImGui::GetTime() - mouseDownTime;
    if(heldTime > ImGui::GetIO().MouseDoubleClickTime) return;

    auto cameraTransformComponent = ExRendererGetters::currentRenderCameraTransform;
    if(cameraTransformComponent == nullptr) return;

    auto mousePosition = Input::GetMousePosition();
    glm::vec2 worldMousePosition = glm::vec2(mousePosition.x, mousePosition.y) + glm::vec2(cameraTransformComponent->position.x, cameraTransformComponent->position.y);

    const auto& entities = *GetSystemEntities();

    if(TrySelectEntity(entities, worldMousePosition)) return;

    // Every entity under the cursor was already selected by a previous click — clear the
    // ignore-list so the stack can be cycled through again instead of getting stuck.
    ignoredEntityIds.clear();

    if(TrySelectEntity(entities, worldMousePosition)) return;

    ElementSelectionController::SetSelected(nullptr);
};

bool EditorClickSystem::TrySelectEntity(const std::vector<std::shared_ptr<EntityCS>>& entities, const glm::vec2& worldMousePosition){
    for(auto entity : entities){
        if(ignoredEntityIds.find(entity->GetId()) != ignoredEntityIds.end()) continue;

        if(CheckBounds(entity, worldMousePosition))
        {
            entitySelection.SetEntitySelected(entity->GetId());
            ignoredEntityIds.insert(entity->GetId());
            return true;
        }
    }

    return false;
};

bool EditorClickSystem::CheckBounds(const std::shared_ptr<EntityCS> entity, const glm::vec2& worldMousePosition) const{
    auto transformComponent = entity->GetComponent<TransformComponent>();
    auto boxColliderComponent = entity->GetComponent<BoxColliderComponent>();
    glm::vec2 entityPosition(transformComponent->position.x, transformComponent->position.y);

    glm::vec2 rectMin = entityPosition + glm::min(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);
    glm::vec2 rectMax = entityPosition + glm::max(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);

    bool clickedInsideRect = worldMousePosition.x >= rectMin.x && worldMousePosition.x <= rectMax.x
        && worldMousePosition.y >= rectMin.y && worldMousePosition.y <= rectMax.y;

    return clickedInsideRect;
};
