#include "WindowSection.h"
#include "../../EditorInterfaceGetters.h"
#include <imgui.h>

void WindowSection::Draw(){
    if(ImGui::BeginMenu("Window"))
    {
        if(ImGui::BeginMenu("Visualization"))
        {
            if(ImGui::MenuItem("Scene View"))
            {
                EditorInterfaceGetters::sceneViewEnabled = true;
            }

            if(ImGui::MenuItem("Game View"))
            {
                EditorInterfaceGetters::gameViewEnabled = true;
            }

            if(ImGui::MenuItem("Asset Browser  [ Space ]"))
            {
                EditorInterfaceGetters::assetBrowserIsOpened = true;
            }

            if(ImGui::MenuItem("Console"))
            {
                EditorInterfaceGetters::consoleEnabled = true;
            }

            if(ImGui::MenuItem("ECS Monitoring Panel"))
            {
                EditorInterfaceGetters::ecsMonitoringEnabled = true;
            }

            if(ImGui::MenuItem("ECS Administrator"))
            {
                EditorInterfaceGetters::ecsAdministratorEnabled = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
};