#include "EditorDrawCameraSafeAreaSystem.h"
#include "../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../Engine/Core/Utils/ColorChannel.h"
#include "../../../Engine/Core/Input/Input.h"
#include "../../Main/EditorInterfaceGetters.h"
#include "../../Main/Windows/EditorWindows/ElementSelectionController.h"
#include "../../Main/Windows/EditorWindows/EntityBrowser/EntityBrowserSelection.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <cmath>

namespace{
    constexpr float HANDLE_RADIUS = 8.0f;
    constexpr float HANDLE_HIT_RADIUS = 11.0f;
    constexpr int CIRCLE_SEGMENTS = 24;
    constexpr float TWO_PI = 6.28318530717958647692f;

    // positionBegin/positionEnd draw lighter than limitBegin/limitEnd so the two rects stay
    // visually distinct when they overlap.
    const ColorChannel POSITION_COLOR(200, 200, 200, 255);
    const ColorChannel LIMIT_COLOR(80, 80, 80, 255);

    void DrawHandleCircle(SDL_Renderer* renderer, const glm::vec2& center, float radius){
        glm::vec2 previousPoint(center.x + radius, center.y);
        for(int i = 1; i <= CIRCLE_SEGMENTS; i++){
            const float angle = (static_cast<float>(i) / CIRCLE_SEGMENTS) * TWO_PI;
            const glm::vec2 point(center.x + radius * std::cos(angle), center.y + radius * std::sin(angle));
            SDL_RenderDrawLine(renderer, static_cast<int>(previousPoint.x), static_cast<int>(previousPoint.y), static_cast<int>(point.x), static_cast<int>(point.y));
            previousPoint = point;
        }
    }

    void DrawHandleGroup(SDL_Renderer* renderer, const glm::vec2& screenBegin, const glm::vec2& screenEnd, const ColorChannel& color){
        const glm::vec2 rectMin = glm::min(screenBegin, screenEnd);
        const glm::vec2 rectMax = glm::max(screenBegin, screenEnd);

        SDL_Rect rect = {
            static_cast<int>(rectMin.x),
            static_cast<int>(rectMin.y),
            static_cast<int>(rectMax.x - rectMin.x),
            static_cast<int>(rectMax.y - rectMin.y)
        };

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 40);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawRect(renderer, &rect);

        DrawHandleCircle(renderer, screenBegin, HANDLE_RADIUS);
        DrawHandleCircle(renderer, screenEnd, HANDLE_RADIUS);
    }
}

EditorDrawCameraSafeAreaSystem::EditorDrawCameraSafeAreaSystem(){
    Require<CameraSafeAreaComponent>(false);
};

void EditorDrawCameraSafeAreaSystem::UpdateSystem(SystemContext systemContext){
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

    int selectedEntityId = -1;
    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    if(currentSelection != nullptr && currentSelection->GetType() == EditorSelectableType::Entity){
        if(auto entitySelection = dynamic_cast<EntityBrowserSelection*>(currentSelection))
            selectedEntityId = entitySelection->GetSelectedEntityId();
    }

    SDL_BlendMode previousBlendMode;
    SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for(auto entity : systemEntities){
        const bool isSelected = selectedEntityId >= 0 && static_cast<unsigned int>(selectedEntityId) == entity->GetId();
        if(!isSelected) continue;

        const auto safeAreaComponent = entity->GetComponent<CameraSafeAreaComponent>();
        if(!safeAreaComponent->showGizmo) continue;

        // The limit box must always be bigger than (contain) the position box:
        // limitBegin <= positionBegin and limitEnd >= positionEnd, component-wise.
        if(isDragging && draggedEntityId == entity->GetId()){
            switch(draggedHandle){
                case SafeAreaHandle::PositionBegin:
                    safeAreaComponent->positionBegin = glm::max(worldMousePosition, safeAreaComponent->limitBegin);
                    break;
                case SafeAreaHandle::PositionEnd:
                    safeAreaComponent->positionEnd = glm::min(worldMousePosition, safeAreaComponent->limitEnd);
                    break;
                case SafeAreaHandle::LimitBegin:
                    safeAreaComponent->limitBegin = glm::min(worldMousePosition, safeAreaComponent->positionBegin);
                    break;
                case SafeAreaHandle::LimitEnd:
                    safeAreaComponent->limitEnd = glm::max(worldMousePosition, safeAreaComponent->positionEnd);
                    break;
            }
        }

        const glm::vec2 screenPositionBegin = ExRendererGetters::WorldToScreen(safeAreaComponent->positionBegin, cameraPosition);
        const glm::vec2 screenPositionEnd = ExRendererGetters::WorldToScreen(safeAreaComponent->positionEnd, cameraPosition);
        const glm::vec2 screenLimitBegin = ExRendererGetters::WorldToScreen(safeAreaComponent->limitBegin, cameraPosition);
        const glm::vec2 screenLimitEnd = ExRendererGetters::WorldToScreen(safeAreaComponent->limitEnd, cameraPosition);

        DrawHandleGroup(renderer, screenLimitBegin, screenLimitEnd, LIMIT_COLOR);
        DrawHandleGroup(renderer, screenPositionBegin, screenPositionEnd, POSITION_COLOR);

        if(!imguiWantsMouse && !isDragging && mouseJustPressed){
            const struct { SafeAreaHandle handle; const glm::vec2& screenPosition; } candidates[] = {
                { SafeAreaHandle::PositionBegin, screenPositionBegin },
                { SafeAreaHandle::PositionEnd, screenPositionEnd },
                { SafeAreaHandle::LimitBegin, screenLimitBegin },
                { SafeAreaHandle::LimitEnd, screenLimitEnd }
            };

            float closestDistance = HANDLE_HIT_RADIUS;
            bool foundHandle = false;
            SafeAreaHandle closestHandle = SafeAreaHandle::PositionBegin;

            for(const auto& candidate : candidates){
                const float distance = glm::length(mouseScreenPosition - candidate.screenPosition);
                if(distance <= closestDistance){
                    closestDistance = distance;
                    closestHandle = candidate.handle;
                    foundHandle = true;
                }
            }

            if(foundHandle){
                isDragging = true;
                draggedEntityId = entity->GetId();
                draggedHandle = closestHandle;
            }
        }
    }

    if(!mousePressed) isDragging = false;

    SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
};
