#include "ExProjectSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include <imgui.h>

void ExProjectSettingsWindow::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::projectSettingsEnabled) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);

    if(ImGui::Begin("Project Settings", &EditorInterfaceGetters::projectSettingsEnabled))
    {
        ImGui::Text("All Project settings will be here");
    }
    ImGui::End();
};