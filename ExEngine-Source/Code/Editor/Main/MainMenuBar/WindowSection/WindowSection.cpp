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

            if(ImGui::MenuItem("Asset Browser"))
            {
                EditorInterfaceGetters::assetBrowserIsOpened = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
};