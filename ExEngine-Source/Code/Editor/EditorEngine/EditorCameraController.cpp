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

EditorCameraController::EditorCameraController(){
    lastMousePos = glm::vec2(0,0);
    transform = std::make_shared<TransformComponent>(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    renderingSystem = ExRenderer::GetRenderingSystem2D();

    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Update(); };
    *EditorCommandEventHandler::focusSelected += [this](){ this->FocusOnSelection(); };
};

void EditorCameraController::Update(){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;

    HandlePan();

    ExRendererGetters::currentRenderCameraTransform = transform;

    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(ExRendererGetters::renderer);

    renderingSystem->UpdateSystem(SystemContext::PRE_RENDER);
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

    int windowWidth, windowHeight;
    SDL_GetWindowSize(ExRendererGetters::window, &windowWidth, &windowHeight);

    transform->position = entityTransform->position - glm::vec3(windowWidth * 0.5f, windowHeight * 0.5f, 0);
};
