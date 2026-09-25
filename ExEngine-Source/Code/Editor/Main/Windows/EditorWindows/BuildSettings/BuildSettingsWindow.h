#pragma once
#include "../../EditorWindow.h"
#include "BuildType.h"
#include "../../../../../Engine/Core/Project/ProjectInfo.h"
#include <imgui.h>
#include <string>
#include <vector>

class BuildSettingsWindow: public EditorWindow{
private:
    bool webGlIsSelected;
    bool pcIsSelected;
    bool androidIsSelected;
    bool iosIsSelected;

    ImVec2 webGlSize;
    ImVec2 pcSize;
    ImVec2 androidSize;
    ImVec2 iosSize;

    ProjectInfo projectInfo;
    std::string defaultWorldSearchFilter;

    bool hasBuildResult = false;
    bool lastBuildSuccess = false;
    std::string lastBuildMessage;

    void LoadProjectInfo();
    void SaveProjectInfo();
    std::vector<std::string> GetAvailableWorldNames() const;
    void DrawDefaultWorldSelector();
    void DrawDevModeToggle();
    void RunBuild();
    void CleanBuild();

public:
    BuildSettingsWindow();
    void Draw(const int phase) override; //0 == early 1 == late
};