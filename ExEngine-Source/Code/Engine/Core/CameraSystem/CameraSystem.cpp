#include "CameraSystem.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../Components/CameraComponent.h"
#include "../Components/TransformComponent.h"
#include "../Utils/Color.h"
#include "NoCameraEventHandler.h"
#include <SDL.h>

CameraSystem::CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem){
    Require<CameraComponent>(false);
    Require<TransformComponent>(false);

    this->renderingSystem = renderingSystem;
};

void CameraSystem::UpdateSystem(SystemContext systemContext){
    auto allEntities = *GetSystemEntities();
    std::shared_ptr<EntityCS> currentCamera = nullptr;
    
    for(auto camera : allEntities){
        if(camera == nullptr) continue;
        if(currentCamera == nullptr)
        {
            currentCamera = camera;
            continue;
        }

        auto currentCameraComponent = currentCamera->GetComponent<CameraComponent>();
        auto nextCameraComponent = camera->GetComponent<CameraComponent>();

        //ToDo Adjust this rule to correctly compare cameras
        //ToDo adjust this to permit multiple cameras in order ( without clear render )
        // ToDo Adjust this to permit overlay cameras ( UI )
        if(currentCameraComponent->display < nextCameraComponent->display)
            currentCamera = camera;
    }
    RenderCamera(currentCamera, systemContext);

#ifndef EXENGINE_EDITOR
    // In the editor, the frame is presented once by EditorInterface after ImGui draws on top.
    SDL_RenderPresent(ExRendererGetters::renderer);
#endif
};

void CameraSystem::RenderCamera(std::shared_ptr<EntityCS> camera, SystemContext systemContext){
    if(camera == nullptr)
    {
        NoCameraEventHandler::noCameraHandler->Invoke();
        return;
    }

    auto cameraComponent = camera->GetComponent<CameraComponent>();
    ExRendererGetters::currentRenderCameraTransform = camera->GetComponent<TransformComponent>();

    //cleaning window with a base color
    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    //ToDo, may it doesn't works with overlay cameras
    SDL_RenderClear(ExRendererGetters::renderer); 

    renderingSystem->UpdateSystem(systemContext);
};