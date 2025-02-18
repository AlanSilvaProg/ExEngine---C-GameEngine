#include "CameraSystem.h"
#include "../Components/CameraComponent.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include <SDL.h>

CameraSystem::CameraSystem(){
    Require<CameraComponent>(false);
};

void CameraSystem::UpdateSystem(){
    for(auto entity : systemEntities){
        auto camera = entity.GetComponent<CameraComponent>();
        SDL_RenderSetScale(ExRendererGetters::renderer, camera->zoom.x, camera->zoom.y);
        SDL_RenderSetLogicalSize(ExRendererGetters::renderer, camera->size.x, camera->size.y);
    };
};