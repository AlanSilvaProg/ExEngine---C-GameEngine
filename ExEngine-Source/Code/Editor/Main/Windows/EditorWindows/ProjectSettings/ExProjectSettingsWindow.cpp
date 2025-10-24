#include "ExProjectSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Configuration/ConfigurationFileManager.h"
#include "../../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include <imgui.h>

void ExProjectSettingsWindow::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::projectSettingsEnabled) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);

    if(ImGui::Begin("Project Settings", &EditorInterfaceGetters::projectSettingsEnabled))
    {
        static bool lastFrameHasEdition = false;
        bool editingSomething = false;
        bool activingSomething = false;

        auto targetFps = RuntimeSettings::GetTargetFps();
        ImGui::DragInt("FPS Limit", &targetFps, 1.0, 1.0, 320);
        if(ImGui::IsItemActive())
        {
            activingSomething = true;
            if(ImGui::IsItemEdited())
            {
                editingSomething = lastFrameHasEdition = true;
                RuntimeSettings::SetTargetFps(targetFps);
            }
        }

        if(!editingSomething && !activingSomething && lastFrameHasEdition)
        {
            lastFrameHasEdition = false;
            ConfigurationFileManager::SaveCurrentState();
        }
    }
    ImGui::End();
};