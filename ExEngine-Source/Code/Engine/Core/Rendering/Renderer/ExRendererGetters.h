#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../../Components/TransformComponent.h"

class ExRendererGetters{
public:
    static SDL_Renderer* renderer;
    static SDL_Window* window;
    static std::shared_ptr<TransformComponent> currentRenderCameraTransform;

    // Resolution management
    static int renderWidth;
    static int renderHeight;

    // Resolution methods
    static void SetRenderResolution(int width, int height);
    static void GetRenderResolution(int& width, int& height);

    static glm::vec2 WorldToScreen(const glm::vec2& worldPosition, const glm::vec2& cameraPosition);
    static glm::vec2 ScreenToWorld(const glm::vec2& screenPosition, const glm::vec2& cameraPosition);
};