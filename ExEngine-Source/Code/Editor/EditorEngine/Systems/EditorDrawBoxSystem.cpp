#include "EditorDrawBoxSystem.h"
#include "../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../Engine/Core/Utils/Color.h"
#include "../../../Engine/Core/Input/Input.h"
#include "../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "../../Main/EditorInterfaceGetters.h"
#include "../../Main/Windows/EditorWindows/ElementSelectionController.h"
#include "../../Main/Windows/EditorWindows/EntityBrowser/EntityBrowserSelection.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <imgui.h>

namespace{
    constexpr float HANDLE_HALF_SIZE = 4.0f;
    constexpr float HANDLE_HIT_HALF_SIZE = 6.0f;
}

EditorDrawBoxSystem::EditorDrawBoxSystem(){
    Require<BoxColliderComponent>(false);
    Require<TransformComponent>(false);
};

void EditorDrawBoxSystem::UpdateSystem(SystemContext systemContext){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;

    auto renderer = ExRendererGetters::renderer;
    if(renderer == nullptr) return;

    auto cameraTransform = ExRendererGetters::currentRenderCameraTransform;
    if(cameraTransform == nullptr) return;

    const glm::vec2 cameraPosition(cameraTransform->position.x, cameraTransform->position.y);

    const glm::ivec2 mousePositionInt = Input::GetMousePosition();
    const glm::vec2 mouseScreenPosition(mousePositionInt.x, mousePositionInt.y);
    const glm::vec2 worldMousePosition = mouseScreenPosition + cameraPosition;

    const bool mousePressed = Input::GetMouseButtonPressed(SDL_BUTTON_LEFT);
    const bool mouseJustPressed = Input::GetMouseButtonDown(SDL_BUTTON_LEFT);
    const bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;

    int selectedEntityId = -1;
    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    if(currentSelection != nullptr && currentSelection->GetType() == EditorSelectableType::Entity){
        if(auto entitySelection = dynamic_cast<EntityBrowserSelection*>(currentSelection))
            selectedEntityId = entitySelection->GetSelectedEntityId();
    }

    const bool drawAllPhysics = RuntimeSettings::GetDrawAllPhysics();

    SDL_BlendMode previousBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    auto fillColor = Color::RED;

    auto entities = systemEntities;

    std::erase_if(entities, [cameraTransform](std::shared_ptr<EntityCS> entity){
        return cameraTransform->position.z > entity->GetComponent<TransformComponent>()->position.z;
    });

    for(auto entity : entities){
        const bool isSelected = selectedEntityId >= 0 && static_cast<unsigned int>(selectedEntityId) == entity->GetId();
        if(!drawAllPhysics && !isSelected) continue;

        const auto transformComponent = entity->GetComponent<TransformComponent>();
        const auto boxColliderComponent = entity->GetComponent<BoxColliderComponent>();

        const glm::vec2 entityPosition(transformComponent->position.x, transformComponent->position.y);

        // Corner being dragged moves with the mouse while the opposite corner (captured on drag start) stays put.
        if(isDragging && draggedEntityId == entity->GetId()){
            const glm::vec2 localMouse = worldMousePosition - entityPosition;
            boxColliderComponent->exRect.beginRect = fixedLocalCorner;
            boxColliderComponent->exRect.endRect = localMouse;
        }

        const glm::vec2 localMin = glm::min(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);
        const glm::vec2 localMax = glm::max(boxColliderComponent->exRect.beginRect, boxColliderComponent->exRect.endRect);

        const glm::vec2 localCorners[4] = {
            glm::vec2(localMin.x, localMin.y),
            glm::vec2(localMax.x, localMin.y),
            glm::vec2(localMin.x, localMax.y),
            glm::vec2(localMax.x, localMax.y)
        };

        glm::vec2 screenCorners[4];
        for(int i = 0; i < 4; i++){
            screenCorners[i] = entityPosition + localCorners[i] - cameraPosition;
        }

        SDL_Rect rect = {
            static_cast<int>(screenCorners[0].x),
            static_cast<int>(screenCorners[0].y),
            static_cast<int>(screenCorners[3].x - screenCorners[0].x),
            static_cast<int>(screenCorners[3].y - screenCorners[0].y)
        };

        SDL_SetRenderDrawColor(renderer, fillColor->r, fillColor->g, fillColor->b, 60);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, fillColor->r, fillColor->g, fillColor->b, 255);
        SDL_RenderDrawRect(renderer, &rect);

        // Resize handles are only shown/interactable on the selected entity, regardless of "Draw all Physics".
        if(isSelected){
            for(int i = 0; i < 4; i++){
                SDL_Rect handleRect = {
                    static_cast<int>(screenCorners[i].x - HANDLE_HALF_SIZE),
                    static_cast<int>(screenCorners[i].y - HANDLE_HALF_SIZE),
                    static_cast<int>(HANDLE_HALF_SIZE * 2.0f),
                    static_cast<int>(HANDLE_HALF_SIZE * 2.0f)
                };

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &handleRect);

                SDL_SetRenderDrawColor(renderer, fillColor->r, fillColor->g, fillColor->b, 255);
                SDL_RenderDrawRect(renderer, &handleRect);
            }

            if(!imguiWantsMouse && !isDragging && mouseJustPressed){
                for(int i = 0; i < 4; i++){
                    const bool insideHandle = mouseScreenPosition.x >= screenCorners[i].x - HANDLE_HIT_HALF_SIZE
                        && mouseScreenPosition.x <= screenCorners[i].x + HANDLE_HIT_HALF_SIZE
                        && mouseScreenPosition.y >= screenCorners[i].y - HANDLE_HIT_HALF_SIZE
                        && mouseScreenPosition.y <= screenCorners[i].y + HANDLE_HIT_HALF_SIZE;

                    if(insideHandle){
                        isDragging = true;
                        draggedEntityId = entity->GetId();
                        fixedLocalCorner = localCorners[3 - i];
                        break;
                    }
                }
            }
        }
    }

    if(!mousePressed) isDragging = false;

    SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
};
