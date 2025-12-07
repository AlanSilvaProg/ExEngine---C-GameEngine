#include "EditorCameraSystem.h"
#include "../Component/EditorCameraComponent.h"
#include "../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../Engine/Core/Components/TransformComponent.h"
#include "../../../Engine/Core/Utils/Color.h"
#include "../../Main/Windows/EditorWindows/SceneView/SceneWindow.h"
#include <SDL.h>

EditorCameraSystem::EditorCameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem){
    Require<EditorCameraComponent>(false);
    Require<TransformComponent>(false);

    this->renderingSystem = renderingSystem;
};

void EditorCameraSystem::UpdateSystem(){
    auto allEntities = *GetSystemEntities();
    auto exRenderer = ExRendererGetters::renderer;
    
    for(auto camera : allEntities){
        ExRendererGetters::currentRenderCamera = camera;

        UpdateDisplayTexture();

        //cleaning window with a base color 
        auto color = Color::BLUE;
        SDL_SetRenderDrawColor(exRenderer, color->r, color->g, color->b, color->a);
        SDL_RenderClear(exRenderer);

        renderingSystem->UpdateSystem();

        DisableDisplayTextures();
    }

    SDL_RenderClear(exRenderer);
};

void EditorCameraSystem::UpdateDisplayTexture(){
#ifdef EXENGINE_EDITOR
    if(SceneWindow::sceneDisplay == nullptr)
    {
        auto sdlTexture = SDL_CreateTexture(ExRendererGetters::renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_TARGET, 800, 800);
        SceneWindow::sceneDisplay = sdlTexture;
        return;
    }
    SDL_SetRenderTarget(ExRendererGetters::renderer, SceneWindow::sceneDisplay);
#endif
};

void EditorCameraSystem::DisableDisplayTextures(){
#ifdef EXENGINE_EDITOR
    if(SceneWindow::sceneDisplay == nullptr) return;
    SDL_SetRenderTarget(ExRendererGetters::renderer, NULL);
#endif
};