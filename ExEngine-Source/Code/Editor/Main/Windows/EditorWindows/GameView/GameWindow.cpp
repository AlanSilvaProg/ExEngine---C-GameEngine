#include "GameWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Utils/Algorithms/ExMath.h"
#include "../../../../../Engine/Core/CameraSystem/CameraSystem.h"
#include "../../../../../Engine/Core/Components/CameraComponent.h"
#include "../../../../../Engine/Core/Components/TransformComponent.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include <glm/glm.hpp>

int GameWindow::targetDisplay = 0;

GameWindow::GameWindow(){
    EditorInterfaceGetters::gameViewEnabled = true; // ToDo -> control with persistence
};

void GameWindow::Draw(int phase) {
    if(phase != 1) return;
    if(!EditorInterfaceGetters::gameViewEnabled) return;
    
    if(!ImGui::Begin("Game", &EditorInterfaceGetters::gameViewEnabled, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
    {
        Logger::LogError("Game editor window openning has been failed");
        return;
    }

    auto canRenderWindow = ExRendererGetters::sceneDisplay.find(targetDisplay) != ExRendererGetters::sceneDisplay.end();

    SDL_Texture* sdlTexture = nullptr;
    int w = 0;
    int h = 0;

    if(canRenderWindow)
    {
        sdlTexture = ExRendererGetters::sceneDisplay[targetDisplay];
        w = 0;
        h = 0;
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
    }

    if(ImGui::BeginMenuBar())
    {
        ImGui::MenuItem("Target Display: ", NULL, false, false);
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        ImGui::InputInt("##display", &targetDisplay);
        if(targetDisplay < 0) targetDisplay = 0;
        ImGui::PopItemWidth();
        ImGui::EndMenuBar();
    }

    if(!ImGui::BeginChild("GameView")){
        ImGui::End();
        return;
    }

    if(!canRenderWindow)
    {
        std::string msg = "No Camera Found with Display index: " + std::to_string(targetDisplay);

        // largura da janela
        auto windowSize = ImGui::GetWindowSize();

        // largura do texto
        auto textSize = ImGui::CalcTextSize(msg.c_str());

        // calcula a posição X para centralizar
        float x = (windowSize.x - textSize.x) * 0.5f;
        float y = (windowSize.y - textSize.y) * 0.5f;

        // define a posição do cursor e desenha
        ImGui::SetCursorPosX(x);
        ImGui::SetCursorPosY(y);
        ImGui::TextColored({1,1,1,1}, "%s", msg.c_str());
        ImGui::EndChild();
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
    ImGui::InvisibleButton("gameView", ImVec2(h == 0 ? 1 : h,w == 0 ? 1 : w), ImGuiButtonFlags_MouseButtonLeft);

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
        }
    }

    ImGui::EndChild();

    ImGui::End();
}; 