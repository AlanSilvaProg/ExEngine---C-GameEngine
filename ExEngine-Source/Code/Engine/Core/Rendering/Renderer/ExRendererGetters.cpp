#include "ExRendererGetters.h"
#include "RendererEvent/ResolutionChangeEventHandler.h"

SDL_Renderer* ExRendererGetters::renderer;
SDL_Window* ExRendererGetters::window;
std::shared_ptr<EntityCS> ExRendererGetters::currentRenderCamera;
std::map<int, SDL_Texture*> ExRendererGetters::sceneDisplay;

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