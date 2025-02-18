#include "SceneWindow.h"
#include "../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../Engine/Logger/Logger.h"
#include <imgui.h>

void SceneWindow::Draw(int phase){
    if(phase != 2) return;
    
    if(ExRendererGetters::sceneDisplay.find(0) == ExRendererGetters::sceneDisplay.end())
    {
        Logger::LogError("No camera to render within the scene editor window");
        return;
    }
    ImGui::Begin("Scene", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);

    auto sdlTexture = ExRendererGetters::sceneDisplay[0];
    int w = 0;
    int h = 0;
    SDL_QueryTexture(sdlTexture, NULL, NULL, &w, &h);
    
    auto viewportGCD = ExMath::GetGCD(w, h);
    auto menuBarHeight = ImGui::GetFrameHeight();
    
    auto sizeW = ImGui::GetContentRegionAvail().x;
    auto sizeH = ImGui::GetContentRegionAvail().y;
    auto aspectW = w / viewportGCD;
    auto aspectH = h / viewportGCD;

    auto limitFactor = sizeW / sizeH;

    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Game", NULL, false, false);
        ImGui::MenuItem(("Aspect Ratio: " + std::to_string(aspectW) + ":" + std::to_string(aspectH)).c_str(), NULL, false, false);
        ImGui::EndMenuBar();
    }

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

    // Calculate the position to center the image
    ImVec2 pos = ImVec2((ImGui::GetWindowSize().x - h) * 0.5f, (ImGui::GetWindowSize().y - w) * 0.5f + menuBarHeight);
    ImGui::SetCursorPos(pos);

    ImGui::Image((ImTextureID)(intptr_t)sdlTexture, ImVec2(w, h));
    ImGui::End();
};