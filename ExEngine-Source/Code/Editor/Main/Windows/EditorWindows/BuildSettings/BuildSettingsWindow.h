#pragma once
#include "../../EditorWindow.h"
#include "BuildType.h"
#include <imgui.h>

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
    
public:
    BuildSettingsWindow();
    void Draw(int phase) override; //0 == early 1 == late
};