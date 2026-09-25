#include "BuildSettingsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../ImGuiUtils.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "../../../../../Engine/File/FileManagement.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../Build/PlayerBuildRunner.h"
#include "../../../../Build/PlayerBuilder.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <algorithm>
#include <cctype>

namespace{
    std::string ToLower(std::string value){
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::tolower(c); });
        return value;
    };
}

BuildSettingsWindow::BuildSettingsWindow(){
    webGlIsSelected = EditorInterfaceGetters::buildTarget == BuildType::WEBGL;
    pcIsSelected = EditorInterfaceGetters::buildTarget == BuildType::PC;
    androidIsSelected = EditorInterfaceGetters::buildTarget == BuildType::ANDROID;
    iosIsSelected = EditorInterfaceGetters::buildTarget == BuildType::IOS;

    LoadProjectInfo();
};

void BuildSettingsWindow::LoadProjectInfo(){
    FileManagement::LoadFromJson(EditorInterfaceGetters::currentProjectPath / "ExProject.exproj", projectInfo);
};

void BuildSettingsWindow::SaveProjectInfo(){
    FileManagement::SaveFile(EditorInterfaceGetters::currentProjectPath / "ExProject.exproj", projectInfo.ToJson().dump());
};

std::vector<std::string> BuildSettingsWindow::GetAvailableWorldNames() const {
    std::vector<std::string> names;

    auto worldsPath = EditorInterfaceGetters::currentProjectPath / "Assets/Worlds";
    if(!std::filesystem::exists(worldsPath)) return names;

    for(const auto& entry : std::filesystem::directory_iterator(worldsPath)){
        if(entry.path().extension() == ".exworld")
            names.push_back(entry.path().stem().string());
    }

    return names;
};

void BuildSettingsWindow::DrawDefaultWorldSelector(){
    ImGui::TextUnformatted("Default World");
    ImGui::TextDisabled("Loaded automatically as soon as a shipped build starts");

    ImGui::InputTextWithHint("##DefaultWorldSearch", "Search worlds...", &defaultWorldSearchFilter);

    ImGui::BeginChild("##DefaultWorldList", ImVec2(0, 120), ImGuiChildFlags_Borders);

    const auto filter = ToLower(defaultWorldSearchFilter);
    for(const auto& worldName : GetAvailableWorldNames()){
        if(!filter.empty() && ToLower(worldName).find(filter) == std::string::npos)
            continue;

        const bool isSelected = projectInfo.defaultWorld == worldName;
        if(ImGui::Selectable(worldName.c_str(), isSelected)){
            projectInfo.defaultWorld = worldName;
            SaveProjectInfo();
        }
    }

    ImGui::EndChild();

    if(!projectInfo.defaultWorld.empty())
        ImGui::TextDisabled("%s", ("Selected: " + projectInfo.defaultWorld).c_str());
};

void BuildSettingsWindow::DrawDevModeToggle(){
    if(ImGui::Checkbox("Dev Mode", &projectInfo.devMode))
        SaveProjectInfo();

    ImGui::TextDisabled(projectInfo.devMode
        ? "Keeps logs and debug symbols"
        : "Release build: no logs, ready to distribute");
};

void BuildSettingsWindow::RunBuild(){
    auto& runner = EditorInterfaceGetters::playerBuildRunner;
    if(runner && runner->IsRunning()) return;

    if(EditorInterfaceGetters::buildTarget == BuildType::ANDROID || EditorInterfaceGetters::buildTarget == BuildType::IOS)
    {
        hasBuildResult = true;
        lastBuildSuccess = false;
        lastBuildMessage = "Android/iOS builds aren't implemented yet.";
        return;
    }

    const char* outputFolder = tinyfd_selectFolderDialog("Select build output folder", nullptr);
    if(outputFolder == nullptr) return;

    hasBuildResult = false;

    auto projectPath = EditorInterfaceGetters::currentProjectPath;
    auto outputPath = std::filesystem::path(outputFolder);
    auto kind = EditorInterfaceGetters::buildTarget == BuildType::WEBGL ? PlayerBuildKind::Web : PlayerBuildKind::Standalone;

    Logger::Log("Build started - this may take a while the first time (building the reusable export template)...");
    runner->Start(kind, projectPath, projectInfo, outputPath, projectInfo.devMode);
};

void BuildSettingsWindow::CleanBuild(){
    auto& runner = EditorInterfaceGetters::playerBuildRunner;
    if(runner && runner->IsRunning()) return;

    if(EditorInterfaceGetters::buildTarget == BuildType::PC || EditorInterfaceGetters::buildTarget == BuildType::WEBGL)
    {
        bool isWeb = EditorInterfaceGetters::buildTarget == BuildType::WEBGL;
        PlayerBuilder::CleanTemplateCache(isWeb, projectInfo.devMode);
    }

    RunBuild();
};

void BuildSettingsWindow::Draw(const int phase){
    if(phase != 1 || !EditorInterfaceGetters::buildWindowEnabled) return;

    auto& runner = EditorInterfaceGetters::playerBuildRunner;
    if(runner && runner->HasResult())
    {
        hasBuildResult = true;
        lastBuildSuccess = runner->ConsumeResult(lastBuildMessage);
    }
    const bool buildRunning = runner && runner->IsRunning();

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

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        DrawDefaultWorldSelector();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        DrawDevModeToggle();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImVec2 b1 = ImGui::CalcTextSize("Build");
        ImVec2 b2 = ImGui::CalcTextSize("Clean Build");

        float totalWidth =
            b1.x + ImGui::GetStyle().FramePadding.x * 2 +
            ImGui::GetStyle().ItemSpacing.x +
            b2.x + ImGui::GetStyle().FramePadding.x * 2;

        float x = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);

        ImGui::BeginDisabled(buildRunning);
        if(ImGui::Button("Build"))
        {
            RunBuild();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(buildRunning);
        if(ImGui::Button("Clean Build"))
        {
            CleanBuild();
        }
        ImGui::EndDisabled();

        if(hasBuildResult)
        {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, lastBuildSuccess ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::TextWrapped("%s", lastBuildMessage.c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::End();
};