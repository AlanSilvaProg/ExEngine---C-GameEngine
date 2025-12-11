#include "WindowSizeManager.h"
#include <algorithm>
#include <cfloat>

void WindowSizeManager::ApplyMinimumSizeConstraint(const char* windowName) {
    // Apply minimum size constraint using ImGui's built-in constraint system
    // This ensures that users cannot resize windows below the minimum size
    ImGui::SetNextWindowSizeConstraints(
        MINIMUM_WINDOW_SIZE,           // Minimum size (100x100)
        ImVec2(FLT_MAX, FLT_MAX)      // Maximum size (unlimited)
    );
}

void WindowSizeManager::SetEngineConfigWindowProperties() {
    ImGui::SetNextWindowSize(ENGINE_CONFIG_INITIAL_SIZE, ImGuiCond_FirstUseEver);
    ApplyMinimumSizeConstraint("Engine Config");
}

ImVec2 WindowSizeManager::GetMinimumWindowSize() {
    return MINIMUM_WINDOW_SIZE;
}

ImVec2 WindowSizeManager::GetEngineConfigInitialSize() {
    return ENGINE_CONFIG_INITIAL_SIZE;
}

ImVec2 WindowSizeManager::ValidateWindowSize(const ImVec2& size) {
    // Clamp the provided size to ensure it meets minimum requirements
    return ImVec2(
        std::max(size.x, MINIMUM_WINDOW_SIZE.x),
        std::max(size.y, MINIMUM_WINDOW_SIZE.y)
    );
}

bool WindowSizeManager::IsValidWindowSize(const ImVec2& size) {
    return (size.x >= MINIMUM_WINDOW_SIZE.x && size.y >= MINIMUM_WINDOW_SIZE.y);
}

void WindowSizeManager::ApplyConstraintWithValidatedSize(const char* windowName, const ImVec2& initialSize, ImGuiCond condition) {
    ApplyMinimumSizeConstraint(windowName);
    
    ImVec2 validatedSize = ValidateWindowSize(initialSize);
    
    ImGui::SetNextWindowSize(validatedSize, condition);
}