#include "BuildSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../ImGuiUtils.h"
#include "../EngineConfig/WindowSizeManager.h"
#include <imgui.h>

BuildSettingsWindow::BuildSettingsWindow(){
    webGlIsSelected = EditorInterfaceGetters::buildTarget == BuildType::WEBGL;
    pcIsSelected = EditorInterfaceGetters::buildTarget == BuildType::PC;
    androidIsSelected = EditorInterfaceGetters::buildTarget == BuildType::ANDROID;
    iosIsSelected = EditorInterfaceGetters::buildTarget == BuildType::IOS;
};

void BuildSettingsWindow::Draw(int phase){
    if(phase != 1 || !EditorInterfaceGetters::buildWindowEnabled) return;

    // Apply minimum size constraint and validate initial size using WindowSizeManager
    WindowSizeManager::ApplyConstraintWithValidatedSize("BuildSettings", ImVec2(400, 600), ImGuiCond_Once);
    ImGui::Begin("BuildSettings", &EditorInterfaceGetters::buildWindowEnabled, ImGuiWindowFlags_NoDocking);
    
    //BuildSettings
    {

#pragma region Plataform Selection

        if(static bool firstOpen = true; firstOpen)
        {
            firstOpen = false;
            webGlSize = ImGui::CalcTextSize("WEB-GL");
            pcSize = ImGui::CalcTextSize("STANDALONE");
            androidSize = ImGui::CalcTextSize("ANDROID");
            iosSize = ImGui::CalcTextSize("IOS");
        }

        pcIsSelected = EditorInterfaceGetters::buildTarget == BuildType::PC;
        iosIsSelected = EditorInterfaceGetters::buildTarget == BuildType::IOS;
        webGlIsSelected = EditorInterfaceGetters::buildTarget == BuildType::WEBGL;
        androidIsSelected = EditorInterfaceGetters::buildTarget == BuildType::ANDROID;

        auto selectablesSize = webGlSize.x + pcSize.x + androidSize.x + iosSize.x + ImGui::GetStyle().ItemSpacing.x * 3;
        float selectablesPosition = (ImGui::GetContentRegionAvail().x - selectablesSize) / 2;

        ImGui::SetCursorPosX(selectablesPosition);

        if(ImGui::Selectable("WEB-GL", &webGlIsSelected, 0, webGlSize))
        {
            if(webGlIsSelected) EditorInterfaceGetters::buildTarget = BuildType::WEBGL;
        }

        IMGUI_SPACE_SAME_LINE

        if(ImGui::Selectable("STANDALONE", &pcIsSelected, 0, pcSize))
        {
            if(pcIsSelected) EditorInterfaceGetters::buildTarget = BuildType::PC;
        }

        IMGUI_SPACE_SAME_LINE
        
        if(ImGui::Selectable("ANDROID", &androidIsSelected, 0, androidSize))
        {
            if(androidIsSelected) EditorInterfaceGetters::buildTarget = BuildType::ANDROID;
        }

        IMGUI_SPACE_SAME_LINE
        
        if(ImGui::Selectable("IOS", &iosIsSelected, 0, iosSize))
        {
            if(iosIsSelected) EditorInterfaceGetters::buildTarget = BuildType::IOS;
        }

#pragma endregion

        ImVec2 b1 = ImGui::CalcTextSize("Build");
        ImVec2 b2 = ImGui::CalcTextSize("Switch Platform");

        float totalWidth =
            b1.x + ImGui::GetStyle().FramePadding.x * 2 +
            ImGui::GetStyle().ItemSpacing.x +
            b2.x + ImGui::GetStyle().FramePadding.x * 2;

        float x = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);

        if(ImGui::Button("Build"))
        {
            
        }

        ImGui::SameLine();

        if(ImGui::Button("Switch Platform"))
        {

        }
    }

    ImGui::End();
};