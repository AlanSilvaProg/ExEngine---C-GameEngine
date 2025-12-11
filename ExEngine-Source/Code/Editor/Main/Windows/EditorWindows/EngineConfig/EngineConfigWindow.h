#pragma once
#include "../../EditorWindow.h"
#include "../../../../../Engine/Core/Utils/ColorChannel.h"
#include "ConfigurationManager.h"
#include "WindowSizeManager.h"

class EngineConfigWindow : public EditorWindow {
private:
    enum class ConfigSection {
        EditorStyle
        // Future sections can be added here
    };
    
    ConfigSection selectedSection;
    bool hasUnsavedChanges;
    float lastChangeTime;
    float lastErrorTime;
    bool showErrorPopup;
    bool isWindowFocused;
    bool shouldClose;
    bool isResizing;
    
    void DrawSideMenu();
    void DrawDetailsPanel();
    void DrawEditorStyleSection();
    void DrawStyleChangeIndicator();
    void DrawErrorFeedback();
    void DrawErrorPopup();
    void HandleWindowBehavior();
    void CheckClickOutside();
    void HandleKeyboardShortcuts();
    void ApplyWindowConstraints();
    bool ValidateStyleInput(float& value, float min, float max, const char* name);
    bool ValidateColorInput(ImVec4& color, const char* name);
    ImVec4 ColorToImVec4(std::shared_ptr<ColorChannel> color);
    
public:
    EngineConfigWindow();
    ~EngineConfigWindow();
    void Draw(int phase) override;
    bool ShouldClose() const;
    
    // Static method for keyboard shortcut handling
    static void HandleGlobalKeyboardShortcuts();
};