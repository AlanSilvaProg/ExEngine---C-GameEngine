#include "SceneWindow.h"
#include "../../../../EditorECS/Component/EditorCameraComponent.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../../Engine/Core/Components/TransformComponent.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRenderer.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include <glm/glm.hpp>

SDL_Texture* SceneWindow::sceneDisplay = nullptr;

SceneWindow::SceneWindow(){
    lastMousePos = ImVec2(0,0);
    EditorInterfaceGetters::sceneViewEnabled = true; // ToDo -> control with persistence
    ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();

    //Creating editor entity for camera

    editorCameraSystem = ecsManager->CreateSystem<EditorCameraSystem>(ExRenderer::GetRenderingSystem2D());
    auto editorCamera = ecsManager->CreateEntity("EditorCamera", true);
    editorCamera->AddComponent<TransformComponent>(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1));
    editorCamera->AddComponent<EditorCameraComponent>();
};

void SceneWindow::Draw(int phase){
    if(phase != 1) return;
    if(!EditorInterfaceGetters::sceneViewEnabled) return;
    
    ecsManager->GetSystem<EditorCameraSystem>()->UpdateSystem();

    if(sceneDisplay == nullptr)
    {
        Logger::LogError("No camera to render within the scene editor window");
        return;
    }
    
    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyMinimumSizeConstraint("Scene");
    
    if(!ImGui::Begin("Scene", &EditorInterfaceGetters::sceneViewEnabled, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
    {
        Logger::LogError("Scene editor window openning has been failed");
        ImGui::End();
        return;
    }

    int w = 0;
    int h = 0;
    SDL_QueryTexture(sceneDisplay, NULL, NULL, &w, &h);
    
    auto viewportGCD = ExMath::GetGCD(w, h);
    
    auto sizeW = ImGui::GetContentRegionAvail().x;
    auto sizeH = ImGui::GetContentRegionAvail().y;
    auto aspectW = w / viewportGCD;
    auto aspectH = h / viewportGCD;

    auto limitFactor = sizeW / sizeH;

    if(sizeW < w || sizeH < h)
    {
        if(limitFactor >= 1){
            h = sizeH;
            w = (h / aspectH) * aspectW;
        }
        else{
            w = sizeW;
            h = (w / aspectW) * aspectH;
        }
    }

    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Game", NULL, false, false);
        ImGui::EndMenuBar();
    }

    if(ImGui::BeginChild("SceneView"))
    {
        // Calculate the position to center the image
        ImVec2 pos = ImVec2((ImGui::GetWindowSize().x - h) * 0.5f, (ImGui::GetWindowSize().y - w) * 0.5f);
        ImVec2 viewportSize(w, h);
        ImGui::SetCursorPos(pos);
        ImGui::Image((ImTextureID)(intptr_t)sceneDisplay, viewportSize);

        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 absolute_pos = ImVec2(window_pos.x + pos.x, window_pos.y + pos.y);
        ImVec2 canvas_p0 = absolute_pos;
        ImVec2 canvas_p1(canvas_p0.x + h, canvas_p0.y + w);

        ImGui::SetCursorPos(pos);
        ImGui::InvisibleButton("sceneView", ImVec2(h == 0 ? 1 : h,w == 0 ? 1 : w), ImGuiButtonFlags_MouseButtonLeft);

        if(ImGui::IsItemDeactivated())
        {
            //click ended
        }

        if(ImGui::IsItemActive()) // held action detection
        {
            ImGuiIO& io = ImGui::GetIO();
            const bool isCtrlOrCmdPressed = io.KeyCtrl || io.KeySuper; // ctrl and cmmd
            const bool isLeftMouseClicked = ImGui::IsMouseDown(0); // left mouse click

            if(isCtrlOrCmdPressed && isLeftMouseClicked)
            {
                const auto currentMousePos = ImGui::GetMousePos();

                if(ImGui::IsMouseClicked(0))
                {
                    lastMousePos = currentMousePos;
                }
                else
                {
                    auto cameraSystem = ecsManager->GetSystem<EditorCameraSystem>();
                    auto cameraEntities = cameraSystem->GetSystemEntities();

                    glm::vec3 mouseMoviment((currentMousePos.x - lastMousePos.x) * -1, (currentMousePos.y - lastMousePos.y) * -1, 0);

                    //ToDo will be something similar to objects on scene
                    for(auto camera : *cameraEntities)
                    {
                        auto cameraTransformComponent = camera->GetComponent<TransformComponent>();
                        cameraTransformComponent->Move(mouseMoviment);
                    }

                    lastMousePos = currentMousePos;
                }
            }
        }
    }
    //Fill scene view with rect
    //ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
    //draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
    //draw_list->PushClipRect(canvas_p0, canvas_p1, true);

    ImGui::EndChild();

    ImGui::End();
};