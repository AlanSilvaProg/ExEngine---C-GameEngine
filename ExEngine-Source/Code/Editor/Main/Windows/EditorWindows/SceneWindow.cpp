#include "SceneWindow.h"
#include "../../EditorInterfaceGetters.h"
#include "../../../../Engine/Logger/Logger.h"
#include "../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../Engine/Core/CameraSystem/CameraSystem.h"
#include "../../../../Engine/Core/Components/CameraComponent.h"
#include "../../../../Engine/Core/Components/TransformComponent.h"
#include "../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include <glm/glm.hpp>

void SceneWindow::Draw(int phase){
    if(phase != 2) return;
    
    if(ExRendererGetters::sceneDisplay.find(0) == ExRendererGetters::sceneDisplay.end())
    {
        Logger::LogError("No camera to render within the scene editor window");
        return;
    }
    
    if(!ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
    {
        Logger::LogError("Scene editor window openning has been failed");
        return;
    }

    auto sdlTexture = ExRendererGetters::sceneDisplay[0];
    int w = 0;
    int h = 0;
    SDL_QueryTexture(sdlTexture, NULL, NULL, &w, &h);
    
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
        ImGui::MenuItem(("Aspect Ratio: " + std::to_string(aspectW) + ":" + std::to_string(aspectH)).c_str(), NULL, false, false);
        ImGui::EndMenuBar();
    }

    if(!ImGui::BeginChild("SceneView")){
        ImGui::End();
        return;
    }

    // Calculate the position to center the image
    ImVec2 pos = ImVec2((ImGui::GetWindowSize().x - h) * 0.5f, (ImGui::GetWindowSize().y - w) * 0.5f);
    ImVec2 viewportSize(w, h);
    ImGui::SetCursorPos(pos);
    ImGui::Image((ImTextureID)(intptr_t)sdlTexture, viewportSize);

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
                auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
                auto cameraSystem = ecsManager->GetSystem<CameraSystem>();
                auto cameraEntities = cameraSystem->GetSystemEntities();

                glm::vec3 mouseMoviment((currentMousePos.x - lastMousePos.x) * -1, (currentMousePos.y - lastMousePos.y) * -1, 0);

                //ToDo will be something similar to objects on scene
                for(auto camera : *cameraEntities)
                {
                    auto cameraTransformComponent = camera.GetComponent<TransformComponent>();
                    cameraTransformComponent->Move(mouseMoviment);
                }

                lastMousePos = currentMousePos;
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