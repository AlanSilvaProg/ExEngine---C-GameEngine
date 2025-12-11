#pragma once
#include <imgui.h>

class WindowSizeManager {
private:
    static constexpr ImVec2 MINIMUM_WINDOW_SIZE = ImVec2(100.0f, 100.0f);
    static constexpr ImVec2 ENGINE_CONFIG_INITIAL_SIZE = ImVec2(400.0f, 600.0f);
    
public:
    static void ApplyMinimumSizeConstraint(const char* windowName = nullptr);
    static void SetEngineConfigWindowProperties();
    static ImVec2 GetMinimumWindowSize();
    static ImVec2 GetEngineConfigInitialSize();
    static ImVec2 ValidateWindowSize(const ImVec2& size);
    static bool IsValidWindowSize(const ImVec2& size);
    static void ApplyConstraintWithValidatedSize(const char* windowName, const ImVec2& initialSize, ImGuiCond condition);
};