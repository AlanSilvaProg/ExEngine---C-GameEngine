#include "ExRendererGetters.h"

SDL_Renderer* ExRendererGetters::renderer;
SDL_Window* ExRendererGetters::window;
std::shared_ptr<EntityCS> ExRendererGetters::currentRenderCamera;
std::map<int, SDL_Texture*> ExRendererGetters::sceneDisplay;