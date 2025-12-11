#pragma once
#include "../../../../../Engine/JsonUtility/IJsonConvertable.h"
#include <imgui.h>
#include <map>
#include <string>

struct StyleConfiguration : public IJsonConvertable {
    std::map<ImGuiCol_, ImVec4> colors;
    
    // Sizes and spacing
    ImVec2 windowPadding;
    float windowRounding;
    float windowBorderSize;
    ImVec2 framePadding;
    float frameRounding;
    float frameBorderSize;
    ImVec2 itemSpacing;
    ImVec2 itemInnerSpacing;
    float indentSpacing;
    float scrollbarSize;
    float scrollbarRounding;
    float grabMinSize;
    float grabRounding;
    float tabRounding;
    float tabBorderSize;
    ImVec2 buttonTextAlign;
    ImVec2 selectableTextAlign;
    float alpha;
    float disabledAlpha;
    
    StyleConfiguration();
    
    // Serialization methods
    virtual nlohmann::json ToJson() override;
    virtual void FromJson(const nlohmann::json& json) override;
    void SaveToFile(const std::string& path);
    void LoadFromFile(const std::string& path);
    
    // Style management methods
    void ApplyToImGui();
    void ResetToDefaults();
    void LoadFromImGui();
};