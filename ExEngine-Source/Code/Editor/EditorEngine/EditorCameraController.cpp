#include "EditorCameraController.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/ExRenderer.h"
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/Core/Utils/Color.h"
#include "../Main/EditorInterfaceGetters.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <SDL.h>

EditorCameraController::EditorCameraController(){
    lastMousePos = glm::vec2(0,0);
    transform = std::make_shared<TransformComponent>(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    renderingSystem = ExRenderer::GetRenderingSystem2D();

    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Update(); };
};

void EditorCameraController::Update(){
    if(EditorInterfaceGetters::viewMode != EditorViewMode::SceneView) return;

    HandlePan();

    ExRendererGetters::currentRenderCameraTransform = transform;

    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(ExRendererGetters::renderer);

    renderingSystem->UpdateSystem();
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
