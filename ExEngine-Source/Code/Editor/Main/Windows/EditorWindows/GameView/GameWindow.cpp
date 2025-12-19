#include "GameWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Core/Runtime/App.h"
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
    
    // Apply minimum size constraint using WindowSizeManager
    WindowSizeManager::ApplyMinimumSizeConstraint("Game");
    
    if(!ImGui::Begin("Game", &EditorInterfaceGetters::gameViewEnabled, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    auto canRenderWindow = ExRendererGetters::sceneDisplay.find(targetDisplay) != ExRendererGetters::sceneDisplay.end();

    SDL_Texture* sdlTexture = nullptr;
    int w = 0;
    int h = 0;

    int textureW = 0;
    int textureH = 0;
    float displayW, displayH;
    
    if(canRenderWindow)
    {
        sdlTexture = ExRendererGetters::sceneDisplay[targetDisplay];
        SDL_QueryTexture(sdlTexture, NULL, NULL, &textureW, &textureH);
        
        auto availableW = ImGui::GetContentRegionAvail().x;
        auto availableH = ImGui::GetContentRegionAvail().y;
        
        // Calculate aspect ratio
        float textureAspect = (float)textureW / (float)textureH;
        float availableAspect = availableW / availableH;
        
        // Fit texture to available space while maintaining aspect ratio
        if (textureAspect > availableAspect) {
            // Texture is wider, fit to width
            displayW = availableW;
            displayH = availableW / textureAspect;
        } else {
            // Texture is taller, fit to height
            displayH = availableH;
            displayW = availableH * textureAspect;
        }

        auto viewportGCD = ExMath::GetGCD(textureW, textureH);
        auto aspectW = textureW / viewportGCD;
        auto aspectH = textureH / viewportGCD;

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

        if(ImGui::SmallButton(App::isPlaying ? "Stop" : "Play"))
        {
            App::isPlaying = !App::isPlaying;

            if(!App::isPlaying) 
            {
                EditorInterfaceGetters::Reload();
            }
        }

        ImGui::EndMenuBar();
    }

    if(!ImGui::BeginChild("GameView")){
        ImGui::EndChild();
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
    ImVec2 pos = ImVec2((ImGui::GetWindowSize().x - displayW) * 0.5f, (ImGui::GetWindowSize().y - displayH) * 0.5f);
    ImVec2 viewportSize(displayW, displayH);
    ImGui::SetCursorPos(pos);
    ImGui::Image((ImTextureID)(intptr_t)sdlTexture, viewportSize);

    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 absolute_pos = ImVec2(window_pos.x + pos.x, window_pos.y + pos.y);
    ImVec2 canvas_p0 = absolute_pos;
    ImVec2 canvas_p1(canvas_p0.x + displayW, canvas_p0.y + displayH);

    ImGui::SetCursorPos(pos);
    ImGui::InvisibleButton("gameView", ImVec2(displayW == 0 ? 1 : displayW, displayH == 0 ? 1 : displayH), ImGuiButtonFlags_MouseButtonLeft);

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