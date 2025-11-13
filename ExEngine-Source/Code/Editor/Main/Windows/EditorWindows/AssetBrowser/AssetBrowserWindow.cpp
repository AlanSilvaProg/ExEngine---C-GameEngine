#include "AssetBrowserWindow.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../../../Engine/Core/Runtime/Time/Time.h"
#include "../../../../../Engine/Core/Input/Input.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>
#include <SDL.h>

void AssetBrowserWindow::Draw(int phase){
    if(phase != 1) return;

    if(Input::GetButtonDown(SDL_KeyCode::SDLK_SPACE))
    {
        EditorInterfaceGetters::assetBrowserIsOpened = !EditorInterfaceGetters::assetBrowserIsOpened;
        cancelInstantly = true;
    }

    //remove this comment if you want to disable this window when hided
    //if(!cancelInstantly && !EditorInterfaceGetters::assetBrowserIsOpened && currentPosition == targetPosition) return;

    int w = 0, h = 0;
    SDL_GetWindowSize(ExRendererGetters::window, &w, &h);

    ImGui::SetNextWindowSizeConstraints({static_cast<float>(w), 200}, {static_cast<float>(w), static_cast<float>(h)});

    if(EditorInterfaceGetters::assetBrowserIsOpened)
    {
        ImGui::Begin("Asset Browser - Press Space to Hide###02131231231", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking);
    }
    else
    {
        ImGui::Begin("Asset Browser - Press Space to Show up###02131231231", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    }

    ImGui::SetWindowSize({static_cast<float>(w), ImGui::GetWindowSize().y}, ImGuiCond_Always);

    if(firstUpdate)
    {
        if(EditorInterfaceGetters::assetBrowserIsOpened)
        {
            targetPosition = static_cast<float>(h) + ImGui::GetWindowSize().y;
        }
        else
        {
            targetPosition = static_cast<float>(h) - ImGui::GetWindowSize().y;
        }

        currentPosition = targetPosition;
        firstUpdate = false;
    }
    else
        UpdatePositionTarget(targetPosition, currentPosition, h);

    ImGui::SetWindowPos({0, currentPosition});

    auto availableSize = ImGui::GetContentRegionAvail();

    float projectWindowSize = 0;
    ImGui::SetNextWindowSize({static_cast<float>(availableSize.x * 0.3), static_cast<float>(availableSize.y)}, ImGuiCond_Always);
    ImGui::BeginChild("ProjectFolderList", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("a");
    projectWindowSize = ImGui::GetWindowSize().x;
    ImGui::EndChild();

    ImGui::SameLine();
    availableSize = ImGui::GetContentRegionAvail();
    
    ImGui::SetNextWindowSize({0, static_cast<float>(availableSize.y)}, ImGuiCond_Always);
    ImGui::BeginChild("ProjectExplorer", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("b");
    ImGui::EndChild();

    ImGui::End();
};

void AssetBrowserWindow::UpdatePositionTarget(float& targetPosition, float& currentPosition, int& h)
{
    auto instantly = false;
    if(!cancelInstantly)
        instantly = targetPosition == currentPosition;

    cancelInstantly = false;

    if (EditorInterfaceGetters::assetBrowserIsOpened)
    {
        targetPosition = static_cast<float>(h) - ImGui::GetWindowSize().y;
    }
    else
    {
        targetPosition = static_cast<float>(h) + ImGui::GetWindowSize().y + 50;
    }

    if(instantly)
    {
        currentPosition = targetPosition;
        return;
    }

    if(currentPosition < targetPosition) 
    {
        currentPosition = currentPosition + ImGui::GetIO().DeltaTime * windowVelocity;
        if(currentPosition > targetPosition) currentPosition = targetPosition;
    }
    if(currentPosition > targetPosition) 
    {
        currentPosition = currentPosition - ImGui::GetIO().DeltaTime * windowVelocity;

        if(currentPosition < targetPosition) currentPosition = targetPosition;
    }
};