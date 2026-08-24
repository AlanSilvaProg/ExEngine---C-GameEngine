#include "WindowSection.h"
#include "../../EditorInterfaceGetters.h"
#include <imgui.h>

void WindowSection::Draw(){
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::BeginMenu("Visualization"))
        {
            ImGui::MenuItem("World Inspection", nullptr, &EditorInterfaceGetters::entityBrowserEnabled);
            ImGui::MenuItem("Asset Browser  [ Space ]", nullptr, &EditorInterfaceGetters::assetBrowserIsOpened);
            ImGui::MenuItem("Console", nullptr, &EditorInterfaceGetters::consoleEnabled);
            ImGui::MenuItem("ECS Monitoring Panel", nullptr, &EditorInterfaceGetters::ecsMonitoringEnabled);
            ImGui::MenuItem("ECS Administrator", nullptr, &EditorInterfaceGetters::ecsAdministratorEnabled);
            ImGui::MenuItem("Engine Config", nullptr, &EditorInterfaceGetters::engineConfigEnabled);
            ImGui::MenuItem("Project Settings", nullptr, &EditorInterfaceGetters::projectSettingsEnabled);
            ImGui::MenuItem("Build Settings", nullptr, &EditorInterfaceGetters::buildWindowEnabled);
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
};