#include "ExRendererGetters.h"

SDL_Renderer* ExRendererGetters::renderer;
SDL_Window* ExRendererGetters::window;
std::shared_ptr<TransformComponent> ExRendererGetters::currentRenderCameraTransform;

glm::vec2 ExRendererGetters::WorldToScreen(const glm::vec2& worldPosition, const glm::vec2& cameraPosition) {
    int width = 0, height = 0;
    if(renderer != nullptr) SDL_GetRendererOutputSize(renderer, &width, &height);

    return worldPosition - cameraPosition + glm::vec2(width * 0.5f, height * 0.5f);
}

glm::vec2 ExRendererGetters::ScreenToWorld(const glm::vec2& screenPosition, const glm::vec2& cameraPosition) {
    int width = 0, height = 0;
    if(renderer != nullptr) SDL_GetRendererOutputSize(renderer, &width, &height);

    return screenPosition - glm::vec2(width * 0.5f, height * 0.5f) + cameraPosition;
}
