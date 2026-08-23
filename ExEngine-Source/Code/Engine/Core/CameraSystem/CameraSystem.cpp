#include "CameraSystem.h"
#include "../Components/CameraComponent.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../Components/TransformComponent.h"
#include "../Utils/Color.h"
#include <SDL.h>

CameraSystem::CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem){
    Require<CameraComponent>(false);
    Require<TransformComponent>(false);

    this->renderingSystem = renderingSystem;
};

void CameraSystem::UpdateSystem(){
    auto allEntities = *GetSystemEntities();
    auto exRenderer = ExRendererGetters::renderer;

    for(auto camera : allEntities){
        ExRendererGetters::currentRenderCamera = camera;

        //cleaning window with a base color
        auto color = Color::BLUE;
        SDL_SetRenderDrawColor(exRenderer, color->r, color->g, color->b, color->a);
        SDL_RenderClear(exRenderer);

        renderingSystem->UpdateSystem();
    }

#ifndef EXENGINE_EDITOR
    // In the editor, the frame is presented once by EditorInterface after ImGui draws on top.
    SDL_RenderPresent(exRenderer);
#endif
};
