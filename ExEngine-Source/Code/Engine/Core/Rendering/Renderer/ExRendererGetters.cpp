#include "ExRendererGetters.h"
#include "RendererEvent/ResolutionChangeEventHandler.h"

SDL_Renderer* ExRendererGetters::renderer;
SDL_Window* ExRendererGetters::window;
std::shared_ptr<TransformComponent> ExRendererGetters::currentRenderCameraTransform;

// Resolution management
int ExRendererGetters::renderWidth = 800;
int ExRendererGetters::renderHeight = 800;

void ExRendererGetters::SetRenderResolution(int width, int height) {
    if (renderWidth != width || renderHeight != height) {
        renderWidth = width;
        renderHeight = height;
        ResolutionChangeEventHandler::NotifyResolutionChange(renderWidth, renderHeight);
    }
}

void ExRendererGetters::GetRenderResolution(int& width, int& height) {
    width = renderWidth;
    height = renderHeight;
}

glm::vec2 ExRendererGetters::WorldToScreen(const glm::vec2& worldPosition, const glm::vec2& cameraPosition) {
    return worldPosition - cameraPosition + glm::vec2(renderWidth * 0.5f, renderHeight * 0.5f);
}

glm::vec2 ExRendererGetters::ScreenToWorld(const glm::vec2& screenPosition, const glm::vec2& cameraPosition) {
    return screenPosition - glm::vec2(renderWidth * 0.5f, renderHeight * 0.5f) + cameraPosition;
}