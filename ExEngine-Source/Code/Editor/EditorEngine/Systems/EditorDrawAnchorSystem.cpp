#include "EditorDrawAnchorSystem.h"
#include "../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../Engine/Core/Utils/Color.h"
#include "../../../Engine/Core/Input/Input.h"
#include "../../Main/EditorInterfaceGetters.h"
#include "../../Main/Windows/EditorWindows/ElementSelectionController.h"
#include "../../Main/Windows/EditorWindows/EntityBrowser/EntityBrowserSelection.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <cmath>

namespace{
    constexpr float RETICLE_RADIUS = 10.0f;
    constexpr float RETICLE_TICK_LENGTH = 6.0f;
    constexpr float RETICLE_HIT_RADIUS = 12.0f;
    constexpr int CIRCLE_SEGMENTS = 24;
    constexpr float TWO_PI = 6.28318530717958647692f;

    void DrawReticleCircle(SDL_Renderer* renderer, const glm::vec2& center, float radius){
        glm::vec2 previousPoint(center.x + radius, center.y);
        for(int i = 1; i <= CIRCLE_SEGMENTS; i++){
            const float angle = (static_cast<float>(i) / CIRCLE_SEGMENTS) * TWO_PI;
            const glm::vec2 point(center.x + radius * std::cos(angle), center.y + radius * std::sin(angle));
            SDL_RenderDrawLine(renderer, static_cast<int>(previousPoint.x), static_cast<int>(previousPoint.y), static_cast<int>(point.x), static_cast<int>(point.y));
            previousPoint = point;
        }
    }
}

EditorDrawAnchorSystem::EditorDrawAnchorSystem(){
    Require<AnchorComponent>(false);
    Require<TransformComponent>(false);
};

void EditorDrawAnchorSystem::UpdateSystem(SystemContext systemContext){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;

    auto renderer = ExRendererGetters::renderer;
    if(renderer == nullptr) return;

    auto cameraTransform = ExRendererGetters::currentRenderCameraTransform;
    if(cameraTransform == nullptr) return;

    const glm::vec2 cameraPosition(cameraTransform->position.x, cameraTransform->position.y);

    const glm::ivec2 mousePositionInt = Input::GetMousePosition();
    const glm::vec2 mouseScreenPosition(mousePositionInt.x, mousePositionInt.y);
    const glm::vec2 worldMousePosition = ExRendererGetters::ScreenToWorld(mouseScreenPosition, cameraPosition);

    const bool mousePressed = Input::GetMouseButtonPressed(SDL_BUTTON_LEFT);
    const bool mouseJustPressed = Input::GetMouseButtonDown(SDL_BUTTON_LEFT);
    const bool imguiWantsMouse = ImGui::GetIO().WantCaptureMouse;

    SDL_BlendMode previousBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    auto reticleColor = Color::YELLOW;

    int selectedEntityId = -1;
    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    if(currentSelection != nullptr && currentSelection->GetType() == EditorSelectableType::Entity){
        if(auto entitySelection = dynamic_cast<EntityBrowserSelection*>(currentSelection))
            selectedEntityId = entitySelection->GetSelectedEntityId();
    }

    auto entities = systemEntities;

    std::erase_if(entities, [cameraTransform](std::shared_ptr<EntityCS> entity){
        return cameraTransform->position.z > entity->GetComponent<TransformComponent>()->position.z;
    });

    for(auto entity : entities){
        const bool isSelected = selectedEntityId >= 0 && static_cast<unsigned int>(selectedEntityId) == entity->GetId();
        if(!isSelected) continue;

        const auto transformComponent = entity->GetComponent<TransformComponent>();
        const auto anchorComponent = entity->GetComponent<AnchorComponent>();

        // RenderingSystem2D draws the sprite's top-left at (transform.position - anchor.position),
        // so the point "anchor.position" units into the sprite always lands on transform.position -
        // that's the pivot the reticle marks and drags. While dragging, move the transform to the
        // mouse and re-derive the anchor from the sprite's fixed top-left (captured on drag start)
        // so the anchor's value changes without the rendered sprite shifting on screen.
        if(isDragging && draggedEntityId == entity->GetId()){
            transformComponent->position.x = worldMousePosition.x;
            transformComponent->position.y = worldMousePosition.y;

            const glm::vec2 newEntityPosition(transformComponent->position.x, transformComponent->position.y);
            const glm::vec2 newAnchorLocal = newEntityPosition - dragStartSpriteTopLeft;
            anchorComponent->position.x = newAnchorLocal.x;
            anchorComponent->position.y = newAnchorLocal.y;
        }

        const glm::vec2 entityPosition(transformComponent->position.x, transformComponent->position.y);
        const glm::vec2 screenPosition = ExRendererGetters::WorldToScreen(entityPosition, cameraPosition);

        SDL_SetRenderDrawColor(renderer, reticleColor->r, reticleColor->g, reticleColor->b, 255);

        DrawReticleCircle(renderer, screenPosition, RETICLE_RADIUS);

        // N/S/E/W ticks sticking out of the circle, like a targeting reticle.
        SDL_RenderDrawLine(renderer, static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y - RETICLE_RADIUS), static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y - RETICLE_RADIUS - RETICLE_TICK_LENGTH));
        SDL_RenderDrawLine(renderer, static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y + RETICLE_RADIUS), static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y + RETICLE_RADIUS + RETICLE_TICK_LENGTH));
        SDL_RenderDrawLine(renderer, static_cast<int>(screenPosition.x - RETICLE_RADIUS), static_cast<int>(screenPosition.y), static_cast<int>(screenPosition.x - RETICLE_RADIUS - RETICLE_TICK_LENGTH), static_cast<int>(screenPosition.y));
        SDL_RenderDrawLine(renderer, static_cast<int>(screenPosition.x + RETICLE_RADIUS), static_cast<int>(screenPosition.y), static_cast<int>(screenPosition.x + RETICLE_RADIUS + RETICLE_TICK_LENGTH), static_cast<int>(screenPosition.y));

        if(!imguiWantsMouse && !isDragging && mouseJustPressed){
            const float distance = glm::length(mouseScreenPosition - screenPosition);
            if(distance <= RETICLE_HIT_RADIUS){
                isDragging = true;
                draggedEntityId = entity->GetId();
                dragStartSpriteTopLeft = entityPosition - glm::vec2(anchorComponent->position.x, anchorComponent->position.y);
            }
        }
    }

    if(!mousePressed) isDragging = false;

    SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
};
