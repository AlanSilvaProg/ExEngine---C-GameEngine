#include "EditorCameraController.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../EditorEvents/EditorCommandEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/ExRenderer.h"
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/Core/Utils/Color.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../Main/Windows/EditorWindows/ElementSelectionController.h"
#include "../Main/Windows/EditorWindows/EntityBrowser/EntityBrowserSelection.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <SDL.h>
#include <algorithm>

namespace{
    constexpr float ZOOM_STEP = 0.1f;
    constexpr float MIN_ZOOM = 0.1f;
    constexpr float MAX_ZOOM = 5.0f;
}

EditorCameraController::EditorCameraController(){
    lastMousePos = glm::vec2(0,0);
    transform = std::make_shared<TransformComponent>(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    renderingSystem = ExRenderer::GetRenderingSystem2D();
    textLabelSystem = ExRenderer::GetTextLabelSystem();

    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Update(); };
    *EditorCommandEventHandler::focusSelected += [this](){ this->FocusOnSelection(); };
};

void EditorCameraController::Update(){
    const bool sceneViewActive = EditorInterfaceGetters::viewMode == EditorViewMode::SceneView;

    // Only one camera pass may drive currentRenderCameraTransform/globalCameraZoom and push into
    // the render queue each frame - the Editor takes exclusive control while the Scene View is
    // open, handing it back to the gameplay camera (CameraSystem) otherwise.
    ExRenderer::SetGameplayCameraEnabled(!sceneViewActive);

    if(!sceneViewActive){
        if(overridingGlobalZoom){
            ExRendererGetters::globalCameraZoom = savedGlobalZoom;
            overridingGlobalZoom = false;
        }
        return;
    }

    HandlePan();
    HandleZoom();

    ExRendererGetters::currentRenderCameraTransform = transform;

    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(ExRendererGetters::renderer);

    if(!overridingGlobalZoom){
        savedGlobalZoom = ExRendererGetters::globalCameraZoom;
        overridingGlobalZoom = true;
    }
    
    ExRendererGetters::globalCameraZoom = EditorInterfaceGetters::editorCameraZoom;

    renderingSystem->UpdateSystem(SystemContext::PRE_RENDER);
    textLabelSystem->UpdateSystem(SystemContext::PRE_RENDER);
};

void EditorCameraController::HandlePan(){
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse) return;

    const bool isCtrlOrCmdPressed = io.KeyCtrl || io.KeySuper;
    if(!isCtrlOrCmdPressed || !ImGui::IsMouseDown(ImGuiMouseButton_Left)) return;

    const auto imguiMousePos = ImGui::GetMousePos();
    const glm::vec2 currentMousePos(imguiMousePos.x, imguiMousePos.y);

    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        lastMousePos = currentMousePos;
        return;
    }

    glm::vec3 mouseMovement((currentMousePos.x - lastMousePos.x) * -1, (currentMousePos.y - lastMousePos.y) * -1, 0);

    transform->Move(mouseMovement);

    lastMousePos = currentMousePos;
};

void EditorCameraController::HandleZoom(){
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse || io.MouseWheel == 0.0f) return;

    float zoom = EditorInterfaceGetters::editorCameraZoom + io.MouseWheel * ZOOM_STEP;
    EditorInterfaceGetters::editorCameraZoom = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
};

void EditorCameraController::FocusOnSelection(){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;

    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    if(currentSelection == nullptr || currentSelection->GetType() != EditorSelectableType::Entity) return;

    auto entitySelection = dynamic_cast<EntityBrowserSelection*>(currentSelection);
    if(entitySelection == nullptr) return;

    auto entity = EditorInterfaceGetters::engine->GetECSManagerPtr()->GetEntity(entitySelection->GetSelectedEntityId());
    if(entity == nullptr) return;

    auto entityTransform = entity->GetComponent<TransformComponent>();
    if(entityTransform == nullptr) return;

    transform->position = entityTransform->position;
};
