#include "EngineConfigWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Core/Utils/Color.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../../../Engine/Core/Configuration/ConfigurationFileManager.h"
#include "../../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <string>

EngineConfigWindow::EngineConfigWindow() {
    selectedSection = ConfigSection::RenderSettings;
    hasUnsavedChanges = false;
    lastChangeTime = 0.0f;
    lastErrorTime = 0.0f;
    showErrorPopup = false;
    isWindowFocused = false;
    shouldClose = false;
    isResizing = false;
    
    ConfigurationManager::Initialize();
}

EngineConfigWindow::~EngineConfigWindow() {
    if (hasUnsavedChanges) {
        ConfigurationManager::SaveStyleConfig();
    }
    
    ConfigurationManager::ClearLastError();
}

void EngineConfigWindow::Draw(int phase) {
    if (phase != 1) return;

    if (!EditorInterfaceGetters::engineConfigEnabled) return;

    HandleKeyboardShortcuts();
    ApplyWindowConstraints();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Engine Config", &EditorInterfaceGetters::engineConfigEnabled, windowFlags)) {
        HandleWindowBehavior();
        DrawErrorFeedback();
        
        ImVec2 availableSpace = ImGui::GetContentRegionAvail();
        float sideMenuWidth = availableSpace.x * 0.25f;
        float detailsPanelWidth = availableSpace.x * 0.75f - ImGui::GetStyle().ItemSpacing.x;
        
        if (ImGui::BeginChild("SideMenu", ImVec2(sideMenuWidth, availableSpace.y), ImGuiChildFlags_Borders)) {
            DrawSideMenu();
        }

        ImGui::EndChild();
        ImGui::SameLine();
        
        if (ImGui::BeginChild("DetailsPanel", ImVec2(detailsPanelWidth, availableSpace.y), ImGuiChildFlags_Borders)) {
            DrawDetailsPanel();
        }
        ImGui::EndChild();
    }
    
    if (shouldClose) {
        EditorInterfaceGetters::engineConfigEnabled = false;
        shouldClose = false;
    }
    
    DrawErrorPopup();
    ImGui::End();
}

void EngineConfigWindow::DrawSideMenu() {
    ImGui::Text("Configuration Sections");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select a configuration section to customize engine settings");
    }
    ImGui::Separator();
    ImGui::Spacing();
    
    bool isRenderSettingsSelected = (selectedSection == ConfigSection::RenderSettings);
    
    if (isRenderSettingsSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
    }
    
    if (ImGui::Selectable("Render Settings", isRenderSettingsSelected, ImGuiSelectableFlags_SpanAllColumns)) {
        selectedSection = ConfigSection::RenderSettings;
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Configure render resolution and camera settings");
    }
    
    if (ImGui::IsItemFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space)) {
            selectedSection = ConfigSection::RenderSettings;
        }
    }
    
    if (isRenderSettingsSelected) {
        ImGui::PopStyleColor(2);
    }
    
    ImGui::Spacing();
    
    bool isEditorStyleSelected = (selectedSection == ConfigSection::EditorStyle);
    
    if (isEditorStyleSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
    }
    
    if (ImGui::Selectable("Editor Style", isEditorStyleSelected, ImGuiSelectableFlags_SpanAllColumns)) {
        selectedSection = ConfigSection::EditorStyle;
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Customize ImGui editor appearance including colors, sizes, and spacing");
    }
    
    if (ImGui::IsItemFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space)) {
            selectedSection = ConfigSection::EditorStyle;
        }
    }
    
    if (isEditorStyleSelected) {
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();

    bool isPreferencesSelected = (selectedSection == ConfigSection::Preferences);

    if (isPreferencesSelected) {
        ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
    }

    if (ImGui::Selectable("Preferences", isPreferencesSelected, ImGuiSelectableFlags_SpanAllColumns)) {
        selectedSection = ConfigSection::Preferences;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Configure editor preferences such as the external text/script editor");
    }

    if (ImGui::IsItemFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space)) {
            selectedSection = ConfigSection::Preferences;
        }
    }

    if (isPreferencesSelected) {
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();

    ImGui::TextDisabled("Keyboard Shortcuts:");
    ImGui::TextDisabled("Ctrl+Shift+E: Toggle window");
}

void EngineConfigWindow::DrawDetailsPanel() {
    switch (selectedSection) {
        case ConfigSection::RenderSettings:
            DrawRenderSettingsSection();
            break;
        case ConfigSection::EditorStyle:
            DrawEditorStyleSection();
            break;
        case ConfigSection::Preferences:
            DrawPreferencesSection();
            break;
        default:
            ImGui::Text("Select a configuration section from the side menu.");
            break;
    }
}

void EngineConfigWindow::DrawEditorStyleSection() {
    ImGui::Text("Editor Style Configuration");
    ImGui::Separator();
    
    StyleConfiguration& style = ConfigurationManager::GetStyleConfig();
    bool styleChanged = false;
    
    if (ImGui::Button("Reset to Defaults")) {
        ConfigurationManager::ResetStyleToDefaults();
        styleChanged = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reset all style settings to ImGui defaults");
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Save Configuration")) {
        if (!ConfigurationManager::SaveStyleConfig()) {
            showErrorPopup = true;
            lastErrorTime = ImGui::GetTime();
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Manually save configuration to file (Ctrl+S)");
    }
    
    ImGui::SameLine();
    
    if (ConfigurationManager::HasError()) {
        if (ImGui::Button("Clear Errors")) {
            ConfigurationManager::ClearLastError();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clear current error messages");
        }
    }
    
    ImGui::Spacing();
    
    if (ImGui::BeginTabBar("StyleTabs")) {
        
        if (ImGui::BeginTabItem("Colors")) {
            ImGui::Text("Window Colors");
            ImGui::Separator();
            
            if (ValidateColorInput(style.colors[ImGuiCol_WindowBg], "Window Background") && 
                ImGui::ColorEdit4("Window Background", (float*)&style.colors[ImGuiCol_WindowBg])) styleChanged = true;
            if (ValidateColorInput(style.colors[ImGuiCol_ChildBg], "Child Background") && 
                ImGui::ColorEdit4("Child Background", (float*)&style.colors[ImGuiCol_ChildBg])) styleChanged = true;
            if (ValidateColorInput(style.colors[ImGuiCol_PopupBg], "Popup Background") && 
                ImGui::ColorEdit4("Popup Background", (float*)&style.colors[ImGuiCol_PopupBg])) styleChanged = true;
            if (ValidateColorInput(style.colors[ImGuiCol_Border], "Border") && 
                ImGui::ColorEdit4("Border", (float*)&style.colors[ImGuiCol_Border])) styleChanged = true;
            if (ValidateColorInput(style.colors[ImGuiCol_BorderShadow], "Border Shadow") && 
                ImGui::ColorEdit4("Border Shadow", (float*)&style.colors[ImGuiCol_BorderShadow])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Frame Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Frame Background", (float*)&style.colors[ImGuiCol_FrameBg])) styleChanged = true;
            if (ImGui::ColorEdit4("Frame Background Hovered", (float*)&style.colors[ImGuiCol_FrameBgHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Frame Background Active", (float*)&style.colors[ImGuiCol_FrameBgActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Title Bar Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Title Background", (float*)&style.colors[ImGuiCol_TitleBg])) styleChanged = true;
            if (ImGui::ColorEdit4("Title Background Active", (float*)&style.colors[ImGuiCol_TitleBgActive])) styleChanged = true;
            if (ImGui::ColorEdit4("Title Background Collapsed", (float*)&style.colors[ImGuiCol_TitleBgCollapsed])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Menu Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Menu Bar Background", (float*)&style.colors[ImGuiCol_MenuBarBg])) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Text & Buttons")) {
            ImGui::Text("Text Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Text", (float*)&style.colors[ImGuiCol_Text])) styleChanged = true;
            if (ImGui::ColorEdit4("Text Disabled", (float*)&style.colors[ImGuiCol_TextDisabled])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Button Colors");
            ImGui::Separator();
            
            // Button colors
            if (ImGui::ColorEdit4("Button", (float*)&style.colors[ImGuiCol_Button])) styleChanged = true;
            if (ImGui::ColorEdit4("Button Hovered", (float*)&style.colors[ImGuiCol_ButtonHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Button Active", (float*)&style.colors[ImGuiCol_ButtonActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Header Colors");
            ImGui::Separator();
            
            // Header colors
            if (ImGui::ColorEdit4("Header", (float*)&style.colors[ImGuiCol_Header])) styleChanged = true;
            if (ImGui::ColorEdit4("Header Hovered", (float*)&style.colors[ImGuiCol_HeaderHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Header Active", (float*)&style.colors[ImGuiCol_HeaderActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Separator Colors");
            ImGui::Separator();
            
            // Separator colors
            if (ImGui::ColorEdit4("Separator", (float*)&style.colors[ImGuiCol_Separator])) styleChanged = true;
            if (ImGui::ColorEdit4("Separator Hovered", (float*)&style.colors[ImGuiCol_SeparatorHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Separator Active", (float*)&style.colors[ImGuiCol_SeparatorActive])) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Selection & Input")) {
            ImGui::Text("Selection Colors");
            ImGui::Separator();
            
            // Selection colors
            if (ImGui::ColorEdit4("Selection Background", (float*)&style.colors[ImGuiCol_TextSelectedBg])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Scrollbar Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Scrollbar Background", (float*)&style.colors[ImGuiCol_ScrollbarBg])) styleChanged = true;
            if (ImGui::ColorEdit4("Scrollbar Grab", (float*)&style.colors[ImGuiCol_ScrollbarGrab])) styleChanged = true;
            if (ImGui::ColorEdit4("Scrollbar Grab Hovered", (float*)&style.colors[ImGuiCol_ScrollbarGrabHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Scrollbar Grab Active", (float*)&style.colors[ImGuiCol_ScrollbarGrabActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Check Mark Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Check Mark", (float*)&style.colors[ImGuiCol_CheckMark])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Slider Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Slider Grab", (float*)&style.colors[ImGuiCol_SliderGrab])) styleChanged = true;
            if (ImGui::ColorEdit4("Slider Grab Active", (float*)&style.colors[ImGuiCol_SliderGrabActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Tab Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Tab", (float*)&style.colors[ImGuiCol_Tab])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Hovered", (float*)&style.colors[ImGuiCol_TabHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Selected", (float*)&style.colors[ImGuiCol_TabSelected])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Selected Overline", (float*)&style.colors[ImGuiCol_TabSelectedOverline])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Dimmed", (float*)&style.colors[ImGuiCol_TabDimmed])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Dimmed Selected", (float*)&style.colors[ImGuiCol_TabDimmedSelected])) styleChanged = true;
            if (ImGui::ColorEdit4("Tab Dimmed Selected Overline", (float*)&style.colors[ImGuiCol_TabDimmedSelectedOverline])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Resize Grip Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Resize Grip", (float*)&style.colors[ImGuiCol_ResizeGrip])) styleChanged = true;
            if (ImGui::ColorEdit4("Resize Grip Hovered", (float*)&style.colors[ImGuiCol_ResizeGripHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Resize Grip Active", (float*)&style.colors[ImGuiCol_ResizeGripActive])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Drag & Drop Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Drag Drop Target", (float*)&style.colors[ImGuiCol_DragDropTarget])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Text Link Colors");
            ImGui::Separator();

            if (ImGui::ColorEdit4("Text Link", (float*)&style.colors[ImGuiCol_TextLink])) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Docking & Tables")) {
            ImGui::Text("Docking Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Docking Preview", (float*)&style.colors[ImGuiCol_DockingPreview])) styleChanged = true;
            if (ImGui::ColorEdit4("Docking Empty Background", (float*)&style.colors[ImGuiCol_DockingEmptyBg])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Table Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Table Header Background", (float*)&style.colors[ImGuiCol_TableHeaderBg])) styleChanged = true;
            if (ImGui::ColorEdit4("Table Border Strong", (float*)&style.colors[ImGuiCol_TableBorderStrong])) styleChanged = true;
            if (ImGui::ColorEdit4("Table Border Light", (float*)&style.colors[ImGuiCol_TableBorderLight])) styleChanged = true;
            if (ImGui::ColorEdit4("Table Row Background", (float*)&style.colors[ImGuiCol_TableRowBg])) styleChanged = true;
            if (ImGui::ColorEdit4("Table Row Background Alt", (float*)&style.colors[ImGuiCol_TableRowBgAlt])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Plot Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Plot Lines", (float*)&style.colors[ImGuiCol_PlotLines])) styleChanged = true;
            if (ImGui::ColorEdit4("Plot Lines Hovered", (float*)&style.colors[ImGuiCol_PlotLinesHovered])) styleChanged = true;
            if (ImGui::ColorEdit4("Plot Histogram", (float*)&style.colors[ImGuiCol_PlotHistogram])) styleChanged = true;
            if (ImGui::ColorEdit4("Plot Histogram Hovered", (float*)&style.colors[ImGuiCol_PlotHistogramHovered])) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Navigation & Modal")) {
            ImGui::Text("Navigation Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Nav Cursor", (float*)&style.colors[ImGuiCol_NavCursor])) styleChanged = true;
            if (ImGui::ColorEdit4("Nav Windowing Highlight", (float*)&style.colors[ImGuiCol_NavWindowingHighlight])) styleChanged = true;
            if (ImGui::ColorEdit4("Nav Windowing Dim Background", (float*)&style.colors[ImGuiCol_NavWindowingDimBg])) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Modal Colors");
            ImGui::Separator();
            
            if (ImGui::ColorEdit4("Modal Window Dim Background", (float*)&style.colors[ImGuiCol_ModalWindowDimBg])) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Sizes & Spacing")) {
            ImGui::Text("Window Properties");
            ImGui::Separator();
            
            if (ValidateStyleInput(style.windowPadding.x, 0.0f, 20.0f, "Window Padding X") &&
                ValidateStyleInput(style.windowPadding.y, 0.0f, 20.0f, "Window Padding Y") &&
                ImGui::SliderFloat2("Window Padding", (float*)&style.windowPadding, 0.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ValidateStyleInput(style.windowRounding, 0.0f, 12.0f, "Window Rounding") &&
                ImGui::SliderFloat("Window Rounding", &style.windowRounding, 0.0f, 12.0f, "%.1f")) styleChanged = true;
            if (ValidateStyleInput(style.windowBorderSize, 0.0f, 1.0f, "Window Border Size") &&
                ImGui::SliderFloat("Window Border Size", &style.windowBorderSize, 0.0f, 1.0f, "%.1f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Frame Properties");
            ImGui::Separator();
            
            // Frame properties
            if (ImGui::SliderFloat2("Frame Padding", (float*)&style.framePadding, 0.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Frame Rounding", &style.frameRounding, 0.0f, 12.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Frame Border Size", &style.frameBorderSize, 0.0f, 1.0f, "%.1f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Item Spacing");
            ImGui::Separator();
            
            if (ImGui::SliderFloat2("Item Spacing", (float*)&style.itemSpacing, 0.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat2("Item Inner Spacing", (float*)&style.itemInnerSpacing, 0.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Indent Spacing", &style.indentSpacing, 0.0f, 30.0f, "%.1f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Scrollbar Properties");
            ImGui::Separator();
            
            if (ImGui::SliderFloat("Scrollbar Size", &style.scrollbarSize, 1.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Scrollbar Rounding", &style.scrollbarRounding, 0.0f, 12.0f, "%.1f")) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Advanced")) {
            ImGui::Text("Grab Properties");
            ImGui::Separator();
            
            // Grab properties
            if (ImGui::SliderFloat("Grab Min Size", &style.grabMinSize, 1.0f, 20.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Grab Rounding", &style.grabRounding, 0.0f, 12.0f, "%.1f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Tab Properties");
            ImGui::Separator();
            
            // Tab properties
            if (ImGui::SliderFloat("Tab Rounding", &style.tabRounding, 0.0f, 12.0f, "%.1f")) styleChanged = true;
            if (ImGui::SliderFloat("Tab Border Size", &style.tabBorderSize, 0.0f, 1.0f, "%.1f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Text Alignment");
            ImGui::Separator();
            
            // Text alignment
            if (ImGui::SliderFloat2("Button Text Align", (float*)&style.buttonTextAlign, 0.0f, 1.0f, "%.2f")) styleChanged = true;
            if (ImGui::SliderFloat2("Selectable Text Align", (float*)&style.selectableTextAlign, 0.0f, 1.0f, "%.2f")) styleChanged = true;
            
            ImGui::Spacing();
            ImGui::Text("Alpha Properties");
            ImGui::Separator();
            
            // Alpha properties with validation
            if (ValidateStyleInput(style.alpha, 0.20f, 1.0f, "Global Alpha") &&
                ImGui::SliderFloat("Global Alpha", &style.alpha, 0.20f, 1.0f, "%.2f")) styleChanged = true;
            if (ValidateStyleInput(style.disabledAlpha, 0.10f, 1.0f, "Disabled Alpha") &&
                ImGui::SliderFloat("Disabled Alpha", &style.disabledAlpha, 0.10f, 1.0f, "%.2f")) styleChanged = true;
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    if (styleChanged) {
        if (ConfigurationManager::ApplyStyleChanges()) {
            hasUnsavedChanges = false; // Changes are automatically saved
            lastChangeTime = ImGui::GetTime();
        } else {
            // Show error feedback if apply failed
            showErrorPopup = true;
            lastErrorTime = ImGui::GetTime();
        }
    }
    
    DrawStyleChangeIndicator();
}

void EngineConfigWindow::DrawStyleChangeIndicator() {
    float timeSinceChange = ImGui::GetTime() - lastChangeTime;
    if (timeSinceChange < 3.0f) { // Show feedback for 3 seconds
        ImGui::Spacing();
        ImGui::Separator();
        
        float alpha = 1.0f - (timeSinceChange / 3.0f);
        ImVec4 feedbackColor = ImVec4(0.0f, 1.0f, 0.0f, alpha);
        
        ImGui::TextColored(feedbackColor, "Style changes applied in real-time!");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, alpha), "Changes are automatically saved and visible across all editor windows.");
        
        ImGui::Spacing();
        if (ImGui::BeginChild("StylePreview", ImVec2(0, 60), ImGuiChildFlags_Borders)) {
            ImGui::Text("Style Preview:");
            ImGui::SameLine();
            if (ImGui::Button("Sample Button")) {
                // Button click action (just for preview)
            }
            ImGui::SameLine();
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_Text], "Sample Text");
        }
        ImGui::EndChild();
    }
}

void EngineConfigWindow::DrawErrorFeedback() {
    if (ConfigurationManager::HasError()) {
        const ConfigurationError& error = ConfigurationManager::GetLastError();
        
        ImVec4 errorColor;
        const char* errorTypeText;
        
        switch (error.type) {
            case ConfigurationError::FileNotFound:
                errorColor = ColorToImVec4(Color::YELLOW);
                errorTypeText = "Warning";
                break;
            case ConfigurationError::FileCorrupted:
            case ConfigurationError::ValidationFailed:
            case ConfigurationError::InvalidData:
                errorColor = ColorToImVec4(Color::RED);
                errorTypeText = "Error";
                break;
            case ConfigurationError::PermissionDenied:
                errorColor = ColorToImVec4(Color::ORANGE);
                errorTypeText = "Permission Error";
                break;
            default:
                errorColor = ColorToImVec4(Color::GREY);
                errorTypeText = "Unknown Error";
                break;
        }
        
        // Draw error banner
        ImGui::PushStyleColor(ImGuiCol_ChildBg, errorColor);
        if (ImGui::BeginChild("ErrorBanner", ImVec2(0, 60), ImGuiChildFlags_Borders)) {
            ImGui::TextColored(ColorToImVec4(Color::WHITE), "%s: %s", errorTypeText, error.message.c_str());
            if (!error.details.empty()) {
                // Light gray for details text
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Details: %s", error.details.c_str());
            }
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 80);
            if (ImGui::Button("Dismiss")) {
                ConfigurationManager::ClearLastError();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
    }
}

void EngineConfigWindow::DrawErrorPopup() {
    if (showErrorPopup && ConfigurationManager::HasError()) {
        ImGui::OpenPopup("Configuration Error");
        showErrorPopup = false;
    }
    
    if (ImGui::BeginPopupModal("Configuration Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const ConfigurationError& error = ConfigurationManager::GetLastError();
        
        ImGui::Text("An error occurred while managing the configuration:");
        ImGui::Separator();
        
        ImGui::TextWrapped("Error: %s", error.message.c_str());
        if (!error.details.empty()) {
            ImGui::TextWrapped("Details: %s", error.details.c_str());
        }
        
        ImGui::Separator();
        
        // Provide action buttons based on error type
        if (error.type == ConfigurationError::FileCorrupted) {
            if (ImGui::Button("Reset to Defaults")) {
                ConfigurationManager::ResetStyleToDefaults();
                ConfigurationManager::ClearLastError();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

bool EngineConfigWindow::ValidateStyleInput(float& value, float min, float max, const char* name) {
    if (std::isnan(value) || std::isinf(value)) {
        // Handle invalid floating point values
        value = (min + max) / 2.0f; // Set to middle of range
        return false;
    }
    
    if (value < min || value > max) {
        // Clamp to valid range
        value = std::clamp(value, min, max);
        return false;
    }
    
    return true;
}

bool EngineConfigWindow::ValidateColorInput(ImVec4& color, const char* name) {
    bool wasValid = true;
    
    // Check for NaN or infinite values
    if (std::isnan(color.x) || std::isinf(color.x)) { color.x = 0.0f; wasValid = false; }
    if (std::isnan(color.y) || std::isinf(color.y)) { color.y = 0.0f; wasValid = false; }
    if (std::isnan(color.z) || std::isinf(color.z)) { color.z = 0.0f; wasValid = false; }
    if (std::isnan(color.w) || std::isinf(color.w)) { color.w = 1.0f; wasValid = false; }
    
    // Clamp to valid range [0.0, 1.0]
    if (color.x < 0.0f || color.x > 1.0f) { color.x = std::clamp(color.x, 0.0f, 1.0f); wasValid = false; }
    if (color.y < 0.0f || color.y > 1.0f) { color.y = std::clamp(color.y, 0.0f, 1.0f); wasValid = false; }
    if (color.z < 0.0f || color.z > 1.0f) { color.z = std::clamp(color.z, 0.0f, 1.0f); wasValid = false; }
    if (color.w < 0.0f || color.w > 1.0f) { color.w = std::clamp(color.w, 0.0f, 1.0f); wasValid = false; }
    
    return wasValid;
}

void EngineConfigWindow::HandleWindowBehavior() {
    isWindowFocused = ImGui::IsWindowFocused();
    
    static bool needsFocus = false;
    static bool wasEnabled = false;
    
    if (EditorInterfaceGetters::engineConfigEnabled && !wasEnabled) {
        needsFocus = true;
        wasEnabled = true;
    } else if (!EditorInterfaceGetters::engineConfigEnabled) {
        wasEnabled = false;
    }
    
    if (needsFocus) {
        ImGui::SetWindowFocus();
        needsFocus = false;
    }
    
    CheckClickOutside();
}

void EngineConfigWindow::CheckClickOutside() {
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        isResizing = true;
        return;
    }
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        isResizing = false;
    }
    
    if (isResizing) {
        return;
    }
    
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
        return;
    }
    
    if (ImGui::IsAnyItemActive()) {
        return;
    }
    
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (hasUnsavedChanges) {
            // Save configuration before closing to prevent data loss
            if (ConfigurationManager::SaveStyleConfig()) {
                hasUnsavedChanges = false;
            }
        }
        shouldClose = true;
    }
}

ImVec4 EngineConfigWindow::ColorToImVec4(std::shared_ptr<ColorChannel> color) {
    return ImVec4(color->r / 255.0f, color->g / 255.0f, color->b / 255.0f, color->a / 255.0f);
}

void EngineConfigWindow::HandleKeyboardShortcuts() {
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) && EditorInterfaceGetters::engineConfigEnabled) {
        if (hasUnsavedChanges) {
            ConfigurationManager::SaveStyleConfig();
            hasUnsavedChanges = false;
        }
        EditorInterfaceGetters::engineConfigEnabled = false;
    }
    
    if (ImGui::IsKeyPressed(ImGuiKey_S) && (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))) {
        if (EditorInterfaceGetters::engineConfigEnabled) {
            ConfigurationManager::SaveStyleConfig();
            hasUnsavedChanges = false;
            lastChangeTime = ImGui::GetTime();
        }
    }
}

void EngineConfigWindow::ApplyWindowConstraints() {
    WindowSizeManager::SetEngineConfigWindowProperties();
}

bool EngineConfigWindow::ShouldClose() const {
    return shouldClose;
}

void EngineConfigWindow::DrawRenderSettingsSection() {
    ImGui::Text("Render Settings Configuration");
    ImGui::Separator();
    
    static int renderWidth, renderHeight;
    static bool initialized = false;
    
    if (!initialized) {
        ExRendererGetters::GetRenderResolution(renderWidth, renderHeight);
        initialized = true;
    }
    
    ImGui::Text("Current Resolution: %dx%d", renderWidth, renderHeight);
    ImGui::Spacing();
    
    // Resolution presets
    ImGui::Text("Resolution Presets:");
    ImGui::Separator();
    
    if (ImGui::Button("720p (1280x720)")) {
        renderWidth = 1280;
        renderHeight = 720;
    }
    ImGui::SameLine();
    if (ImGui::Button("1080p (1920x1080)")) {
        renderWidth = 1920;
        renderHeight = 1080;
    }
    ImGui::SameLine();
    if (ImGui::Button("1440p (2560x1440)")) {
        renderWidth = 2560;
        renderHeight = 1440;
    }
    
    if (ImGui::Button("4K (3840x2160)")) {
        renderWidth = 3840;
        renderHeight = 2160;
    }
    ImGui::SameLine();
    if (ImGui::Button("Square 800x800")) {
        renderWidth = 800;
        renderHeight = 800;
    }
    ImGui::SameLine();
    if (ImGui::Button("Square 1024x1024")) {
        renderWidth = 1024;
        renderHeight = 1024;
    }
    
    ImGui::Spacing();
    ImGui::Text("Custom Resolution:");
    ImGui::Separator();
    
    // Custom resolution inputs
    ImGui::PushItemWidth(150);
    ImGui::InputInt("Width", &renderWidth);
    ImGui::SameLine();
    ImGui::InputInt("Height", &renderHeight);
    ImGui::PopItemWidth();
    
    // Clamp values to reasonable ranges
    if (renderWidth < 320) renderWidth = 320;
    if (renderWidth > 7680) renderWidth = 7680;
    if (renderHeight < 240) renderHeight = 240;
    if (renderHeight > 4320) renderHeight = 4320;
    
    ImGui::Spacing();
    
    // Apply button
    int currentWidth, currentHeight;
    ExRendererGetters::GetRenderResolution(currentWidth, currentHeight);
    
    bool resolutionChanged = (renderWidth != currentWidth || renderHeight != currentHeight);
    
    if (resolutionChanged) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
    }
    
    if (ImGui::Button("Apply Resolution")) {
        ExRendererGetters::SetRenderResolution(renderWidth, renderHeight);
        lastChangeTime = ImGui::GetTime();
    }
    
    if (resolutionChanged) {
        ImGui::PopStyleColor(3);
    }
    
    if (ImGui::IsItemHovered()) {
        if (resolutionChanged) {
            ImGui::SetTooltip("Apply new resolution: %dx%d", renderWidth, renderHeight);
        } else {
            ImGui::SetTooltip("Resolution is already set to %dx%d", renderWidth, renderHeight);
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Reset to Default")) {
        renderWidth = 800;
        renderHeight = 800;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Information section
    ImGui::Text("Resolution Information:");
    ImGui::BulletText("Resolution affects all camera outputs and SDL window");
    ImGui::BulletText("Changes are applied immediately to all systems");
    ImGui::BulletText("Scene and Game windows will automatically adjust");
    ImGui::BulletText("Minimum: 320x240, Maximum: 7680x4320");
    
    float aspectRatio = (float)renderWidth / (float)renderHeight;
    ImGui::Text("Aspect Ratio: %.3f:1", aspectRatio);
    
    // Show feedback when resolution changes
    float timeSinceChange = ImGui::GetTime() - lastChangeTime;
    if (timeSinceChange < 3.0f) {
        ImGui::Spacing();
        ImGui::Separator();
        
        float alpha = 1.0f - (timeSinceChange / 3.0f);
        ImVec4 feedbackColor = ImVec4(0.0f, 1.0f, 0.0f, alpha);
        
        ImGui::TextColored(feedbackColor, "Resolution applied successfully!");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, alpha), "All camera systems and windows have been updated.");
    }
}

void EngineConfigWindow::DrawPreferencesSection() {
    ImGui::Text("Editor Preferences");
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
        lastChangeTime = ImGui::GetTime();
    }
}

void EngineConfigWindow::HandleGlobalKeyboardShortcuts() {
    if (ImGui::IsKeyPressed(ImGuiKey_E) && 
        (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) &&
        (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))) {
        
        EditorInterfaceGetters::engineConfigEnabled = !EditorInterfaceGetters::engineConfigEnabled;
    }
}