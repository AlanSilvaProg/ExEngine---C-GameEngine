#include "ToolbarWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>

void ToolbarWindow::Draw(const int phase){
    if(phase != 1) return;

    ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);

    if(!ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
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
