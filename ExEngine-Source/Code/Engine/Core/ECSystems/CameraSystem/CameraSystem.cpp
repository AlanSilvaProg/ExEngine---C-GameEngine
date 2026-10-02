#include "CameraSystem.h"
#include "../../Rendering/Renderer/ExRendererGetters.h"
#include "../../Components/Rendering/CameraComponent.h"
#include "../../Components/Core/TransformComponent.h"
#include "../../Components/Rendering/CameraSafeAreaComponent.h"
#include "../../Utils/Color.h"
#include "../../CameraSystem/NoCameraEventHandler.h"
#include "../../Utils/Transform/TransformUtils.h"
#include <SDL.h>
#include <glm/glm.hpp>
#include <algorithm>

CameraSystem::CameraSystem(std::shared_ptr<RenderingSystem2D> renderingSystem, std::shared_ptr<TextLabelSystem> textLabelSystem){
    Require<CameraComponent>(false);
    Require<TransformComponent>(false);
    Require<CameraSafeAreaComponent>(true);

    this->renderingSystem = renderingSystem;
    this->textLabelSystem = textLabelSystem;
    resolvedCameraTransform = std::make_shared<TransformComponent>();
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
    auto cameraLocalTransform = camera->GetComponent<TransformComponent>();

    // Camera entities are usually root-level, but if one has a parent its position needs to be
    // composed through it too - Z stays local, it's only used for depth/occlusion, not part of
    // the 2D hierarchy composition.
    auto worldTransform = TransformUtils::GetWorldTransform(camera);
    resolvedCameraTransform->position = glm::vec3(worldTransform.position.x, worldTransform.position.y, cameraLocalTransform->position.z);
    resolvedCameraTransform->rotation = glm::vec3(worldTransform.rotation, cameraLocalTransform->rotation.y, cameraLocalTransform->rotation.z);
    resolvedCameraTransform->scale = glm::vec3(worldTransform.scale.x, worldTransform.scale.y, cameraLocalTransform->scale.z);

    ExRendererGetters::currentRenderCameraTransform = resolvedCameraTransform;

    //Needed to guarantee the expected visualization
    std::shared_ptr<CameraSafeAreaComponent> cameraSafePtr = nullptr;
    if(camera->HasComponent<CameraSafeAreaComponent>(cameraSafePtr)){
        RecalculateZoom(cameraSafePtr);
    }

    //cleaning window with a base color
    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    //ToDo, may it doesn't works with overlay cameras
    SDL_RenderClear(ExRendererGetters::renderer); 

    renderingSystem->UpdateSystem(systemContext);
    textLabelSystem->UpdateSystem(systemContext);
};

void CameraSystem::RecalculateZoom(std::shared_ptr<CameraSafeAreaComponent>& cameraSafeAreaComponent){
    auto renderer = ExRendererGetters::renderer;
    auto cameraTransform = ExRendererGetters::currentRenderCameraTransform;
    if(renderer == nullptr || cameraTransform == nullptr) return;

    int width = 0, height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    if(width <= 0 || height <= 0) return;

    const glm::vec2 cameraPosition(cameraTransform->position.x, cameraTransform->position.y);
    const glm::vec2 halfScreen(width * 0.5f, height * 0.5f);

    const glm::vec2 limitMin = glm::min(cameraSafeAreaComponent->limitBegin, cameraSafeAreaComponent->limitEnd);
    const glm::vec2 limitMax = glm::max(cameraSafeAreaComponent->limitBegin, cameraSafeAreaComponent->limitEnd);

    constexpr float MIN_HALF_EXTENT = 0.0001f;
    const glm::vec2 allowedHalfExtent = glm::max(
        glm::min(cameraPosition - limitMin, limitMax - cameraPosition),
        glm::vec2(MIN_HALF_EXTENT)
    );

    const glm::vec2 zoomToReachAllowedExtent = halfScreen / allowedHalfExtent;
    const float targetZoom = std::max(zoomToReachAllowedExtent.x, zoomToReachAllowedExtent.y);

    ExRendererGetters::globalCameraZoom = std::max(targetZoom, MIN_HALF_EXTENT);
};