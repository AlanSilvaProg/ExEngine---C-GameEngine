#include "ToolboxWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>

namespace{

struct ToolboxEntry{
    const char* label;
    bool* enabledFlag;
};

// Mirrors the "Window" menu list in WindowSection.cpp - same flags, same order.
const ToolboxEntry kToolboxEntries[] = {
    { "World Inspection", &EditorInterfaceGetters::entityBrowserEnabled },
    { "Asset Browser", &EditorInterfaceGetters::assetBrowserIsOpened },
    { "Console", &EditorInterfaceGetters::consoleEnabled },
    { "ECS Monitoring Panel", &EditorInterfaceGetters::ecsMonitoringEnabled },
    { "ECS Administrator", &EditorInterfaceGetters::ecsAdministratorEnabled },
    { "Engine Config", &EditorInterfaceGetters::engineConfigEnabled },
    { "Animation Editor", &EditorInterfaceGetters::animationEditorEnabled },
    { "Project Settings", &EditorInterfaceGetters::projectSettingsEnabled },
    { "Build Settings", &EditorInterfaceGetters::buildWindowEnabled },
    { "Network Test", &EditorInterfaceGetters::networkTestWindowEnabled },
};

}

void ToolboxWindow::Draw(const int phase){
    if(phase != 1) return;

    // Glued to the left edge, right below the main menu bar - ImGui shrinks the main
    // viewport's work area by the menu bar's height, so WorkPos.y already points just past it.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar;

    if(!ImGui::Begin("Toolbox", nullptr, windowFlags))
    {
        ImGui::End();
        return;
    }

    if(!collapsed)
        DrawWindowButtons();

    DrawCollapseToggleBar(lastKnownColumnWidth);

    ImGui::End();
};

void ToolboxWindow::DrawWindowButtons(){
    const float buttonWidth = 180.0f;

    for(const auto& entry : kToolboxEntries)
    {
        const bool isOpen = *entry.enabledFlag;

        if(isOpen)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));

        if(ImGui::Button(entry.label, ImVec2(buttonWidth, 0)))
            *entry.enabledFlag = !isOpen;

        if(isOpen)
            ImGui::PopStyleColor();
    }

    lastKnownColumnWidth = buttonWidth;

    ImGui::Spacing();
};

void ToolboxWindow::DrawCollapseToggleBar(float width){
    const float barHeight = 6.0f;
    const float barWidth = width > 0.0f ? width : 32.0f;

    ImGui::InvisibleButton("##ToolboxCollapseToggle", ImVec2(barWidth, barHeight));
    if(ImGui::IsItemClicked())
        collapsed = !collapsed;

    const ImVec2 barMin = ImGui::GetItemRectMin();
    const ImVec2 barMax = ImGui::GetItemRectMax();
    const ImU32 barColor = ImGui::IsItemHovered() ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Button);
    ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, barColor, barHeight * 0.5f);
};
