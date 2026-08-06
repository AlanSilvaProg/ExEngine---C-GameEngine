#include "ExProjectSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/Configuration/ConfigurationFileManager.h"
#include "../../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <string>

void ExProjectSettingsWindow::Draw(int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::projectSettingsEnabled) return;

    // Apply minimum size constraint and validate initial size using WindowSizeManager
    WindowSizeManager::ApplyConstraintWithValidatedSize("Project Settings", ImVec2(600, 400), ImGuiCond_Once);

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

        ImGui::Separator();
        ImGui::Text("External Text/Script Editor");
        ImGui::TextWrapped("Program used to open scripts and text files (.h, .hpp, .cpp, etc). Leave empty to use the machine's default program for the file type.");

        static std::string externalEditorPath = RuntimeSettings::GetExternalTextEditorPath();
        bool externalEditorChanged = false;

        ImGui::PushItemWidth(-140);
        ImGui::InputText("##ExternalTextEditorPath", &externalEditorPath);
        ImGui::PopItemWidth();
        if(ImGui::IsItemDeactivatedAfterEdit())
        {
            externalEditorChanged = true;
        }

        ImGui::SameLine();
        if(ImGui::Button("Browse..."))
        {
            const char* selected = tinyfd_openFileDialog("Select external editor", "", 0, nullptr, nullptr, 0);
            if(selected != nullptr)
            {
                externalEditorPath = selected;
                externalEditorChanged = true;
            }
        }

        if(!externalEditorPath.empty())
        {
            ImGui::SameLine();
            if(ImGui::Button("Use System Default"))
            {
                externalEditorPath.clear();
                externalEditorChanged = true;
            }
        }

        if(externalEditorChanged)
        {
            RuntimeSettings::SetExternalTextEditorPath(externalEditorPath);
            ConfigurationFileManager::SaveCurrentState();
        }
    }
    ImGui::End();
};