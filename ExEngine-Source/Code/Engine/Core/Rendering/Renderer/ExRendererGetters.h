#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "RenderQueue/IRenderElement.h"
#include "../../Components/Core/TransformComponent.h"
#include <memory>

class ExRendererGetters{
public:
    static SDL_Renderer* renderer;
    static SDL_Window* window;
    static std::shared_ptr<TransformComponent> currentRenderCameraTransform;
    static float globalCameraZoom;

    //internally controlled by ExRenderer
    static std::vector<std::shared_ptr<IRenderElement>> currentRenderQueue;

    static void AddToRenderQueue(std::shared_ptr<IRenderElement> renderElement);

    static const bool ShouldOcclude(const std::shared_ptr<EntityCS>& entity);

    // World<->screen conversion. The camera's position is the world point that lands on the
    // center of the screen (not the top-left corner), so every conversion re-centers around half
    // the window's actual current drawable size - the window is always full canvas, there's no
    // separate configurable render resolution to track. globalCameraZoom scales distance from that
    // center, so zooming stays anchored on the camera instead of drifting toward the screen corner.
    static glm::vec2 WorldToScreen(const glm::vec2& worldPosition, const glm::vec2& cameraPosition);
    static glm::vec2 ScreenToWorld(const glm::vec2& screenPosition, const glm::vec2& cameraPosition);
};