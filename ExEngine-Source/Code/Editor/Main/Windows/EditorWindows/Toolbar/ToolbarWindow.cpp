#include "ToolbarWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>

void ToolbarWindow::Draw(const int phase){
    if(phase != 1) return;

    // Glued to the right edge, right below the main menu bar - ImGui shrinks the main
    // viewport's work area by the menu bar's height, so WorkPos.y already points just past it.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float rightX = viewport->WorkPos.x + viewport->WorkSize.x;
    ImGui::SetNextWindowPos(ImVec2(rightX, viewport->WorkPos.y), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize;

    if(!ImGui::Begin("Toolbar", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }

    const bool isGameView = EditorInterfaceGetters::viewMode == EditorViewMode::GameView;
    const bool isSceneView = EditorInterfaceGetters::viewMode == EditorViewMode::SceneView;

    ImGui::BeginDisabled(isGameView);
    if(ImGui::Button("Game View"))
    {
        EditorInterfaceGetters::viewMode = EditorViewMode::GameView;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(isSceneView);
    if(ImGui::Button("Scene View"))
    {
        EditorInterfaceGetters::viewMode = EditorViewMode::SceneView;
    }
    ImGui::EndDisabled();

    ImGui::End();
};
