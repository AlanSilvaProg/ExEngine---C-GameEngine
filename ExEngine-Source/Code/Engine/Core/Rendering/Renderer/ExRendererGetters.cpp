#include "ExRendererGetters.h"

SDL_Renderer* ExRendererGetters::renderer;
SDL_Window* ExRendererGetters::window;
std::shared_ptr<TransformComponent> ExRendererGetters::currentRenderCameraTransform;
float ExRendererGetters::globalCameraZoom = 1;
std::vector<std::shared_ptr<IRenderElement>> ExRendererGetters::currentRenderQueue;

void ExRendererGetters::AddToRenderQueue(std::shared_ptr<IRenderElement> renderElement){
    currentRenderQueue.push_back(renderElement);
};

const bool ExRendererGetters::ShouldOcclude(const std::shared_ptr<EntityCS>& entity){
    return currentRenderCameraTransform->position.z > entity->GetComponent<TransformComponent>()->position.z;
};

glm::vec2 ExRendererGetters::WorldToScreen(const glm::vec2& worldPosition, const glm::vec2& cameraPosition) {
    int width = 0, height = 0;
    if(renderer != nullptr) SDL_GetRendererOutputSize(renderer, &width, &height);

    glm::vec2 screenCenter(width * 0.5f, height * 0.5f);
    return (worldPosition - cameraPosition) * globalCameraZoom + screenCenter;
}

glm::vec2 ExRendererGetters::ScreenToWorld(const glm::vec2& screenPosition, const glm::vec2& cameraPosition) {
    int width = 0, height = 0;
    if(renderer != nullptr) SDL_GetRendererOutputSize(renderer, &width, &height);

    glm::vec2 screenCenter(width * 0.5f, height * 0.5f);
    return (screenPosition - screenCenter) / globalCameraZoom + cameraPosition;
}
