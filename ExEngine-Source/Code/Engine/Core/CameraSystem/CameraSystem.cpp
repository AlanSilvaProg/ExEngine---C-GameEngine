#include "CameraSystem.h"
#include "../Components/CameraComponent.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../Rendering/Renderer/RendererEvent/ResolutionChangeEventHandler.h"
#include "../Components/TransformComponent.h"
#include "../Utils/Color.h"
#include <SDL.h>
#include <set>

CameraSystem::CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem){
    Require<CameraComponent>(false);
    Require<TransformComponent>(false);

    this->renderingSystem = renderingSystem;
    
    // Register for resolution change events
    if(ResolutionChangeEventHandler::resolutionChangeHandler != nullptr) {
        *ResolutionChangeEventHandler::resolutionChangeHandler += [this](int width, int height) {
            this->OnResolutionChanged(width, height);
        };
    }
};

void CameraSystem::UpdateSystem(){
    DisableAllDisplayTextures();

    auto allEntities = *GetSystemEntities();
    auto exRenderer = ExRendererGetters::renderer;

    for(auto camera : allEntities){
        ExRendererGetters::currentRenderCamera = camera;
        auto display = camera->GetComponent<CameraComponent>()->display;

        UpdateDisplayTexture(display);

        //cleaning window with a base color 
        auto color = Color::BLUE;
        SDL_SetRenderDrawColor(exRenderer, color->r, color->g, color->b, color->a);
        SDL_RenderClear(exRenderer);

        renderingSystem->UpdateSystem();

        DisableDisplayTextures(display);
    }
    SDL_RenderPresent(exRenderer);
};

void CameraSystem::UpdateDisplayTexture(int displayIndex){
#ifdef EXENGINE_EDITOR
    if(ExRendererGetters::sceneDisplay.find(displayIndex) == ExRendererGetters::sceneDisplay.end())
    {
        int renderWidth, renderHeight;
        ExRendererGetters::GetRenderResolution(renderWidth, renderHeight);
        auto sdlTexture = SDL_CreateTexture(ExRendererGetters::renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_TARGET, renderWidth, renderHeight);
        ExRendererGetters::sceneDisplay.insert_or_assign(displayIndex, sdlTexture);
        return;
    }
    SDL_SetRenderTarget(ExRendererGetters::renderer, ExRendererGetters::sceneDisplay[displayIndex]);
#endif
};

void CameraSystem::DisableDisplayTextures(int displayIndex){
#ifdef EXENGINE_EDITOR
    if(ExRendererGetters::sceneDisplay.find(displayIndex) == ExRendererGetters::sceneDisplay.end()) return;
    SDL_SetRenderTarget(ExRendererGetters::renderer, NULL);
#endif
};

void CameraSystem::DisableAllDisplayTextures(){
#ifdef EXENGINE_EDITOR
    auto allEntities = *GetSystemEntities();

    std::set<int> usedDisplays;
    for (auto& camera : allEntities) {
        auto camComp = camera->GetComponent<CameraComponent>();
        if (camComp) {
            usedDisplays.insert(camComp->display);
        }
    }

    auto& sceneDisplay = ExRendererGetters::sceneDisplay;
    for (auto it = sceneDisplay.begin(); it != sceneDisplay.end(); ) {
        if (usedDisplays.find(it->first) == usedDisplays.end()) {
            if (it->second != nullptr) SDL_DestroyTexture(it->second);
            it = sceneDisplay.erase(it);
        } else {
            ++it;
        }
    }
#endif
};

void CameraSystem::OnResolutionChanged(int width, int height) {
#ifdef EXENGINE_EDITOR
    // Recreate all display textures with new resolution
    for (auto& pair : ExRendererGetters::sceneDisplay) {
        if (pair.second != nullptr) {
            SDL_DestroyTexture(pair.second);
            pair.second = SDL_CreateTexture(ExRendererGetters::renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_TARGET, width, height);
        }
    }
#endif
};