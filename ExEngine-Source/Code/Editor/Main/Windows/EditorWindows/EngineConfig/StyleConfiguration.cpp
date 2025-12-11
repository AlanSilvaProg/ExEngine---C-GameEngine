#include "StyleConfiguration.h"
#include "../../../../../Engine/File/FileManagement.h"
#include "../../../../../Engine/Logger/Logger.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

StyleConfiguration::StyleConfiguration() {
    LoadFromImGui();
}

void StyleConfiguration::LoadFromImGui() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Load colors
    colors.clear();
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
        colors[static_cast<ImGuiCol_>(i)] = style.Colors[i];
    }
    
    windowPadding = style.WindowPadding;
    windowRounding = style.WindowRounding;
    windowBorderSize = style.WindowBorderSize;
    framePadding = style.FramePadding;
    frameRounding = style.FrameRounding;
    frameBorderSize = style.FrameBorderSize;
    itemSpacing = style.ItemSpacing;
    itemInnerSpacing = style.ItemInnerSpacing;
    indentSpacing = style.IndentSpacing;
    scrollbarSize = style.ScrollbarSize;
    scrollbarRounding = style.ScrollbarRounding;
    grabMinSize = style.GrabMinSize;
    grabRounding = style.GrabRounding;
    tabRounding = style.TabRounding;
    tabBorderSize = style.TabBorderSize;
    buttonTextAlign = style.ButtonTextAlign;
    selectableTextAlign = style.SelectableTextAlign;
    alpha = style.Alpha;
    disabledAlpha = style.DisabledAlpha;
}

void StyleConfiguration::ApplyToImGui() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Apply colors - ensure all colors are properly set
    for (const auto& colorPair : colors) {
        if (colorPair.first >= 0 && colorPair.first < ImGuiCol_COUNT) {
            style.Colors[colorPair.first] = colorPair.second;
        }
    }
    
    // Apply sizes and spacing with validation
    style.WindowPadding = windowPadding;
    style.WindowRounding = windowRounding;
    style.WindowBorderSize = windowBorderSize;
    style.FramePadding = framePadding;
    style.FrameRounding = frameRounding;
    style.FrameBorderSize = frameBorderSize;
    style.ItemSpacing = itemSpacing;
    style.ItemInnerSpacing = itemInnerSpacing;
    style.IndentSpacing = indentSpacing;
    style.ScrollbarSize = scrollbarSize;
    style.ScrollbarRounding = scrollbarRounding;
    style.GrabMinSize = grabMinSize;
    style.GrabRounding = grabRounding;
    style.TabRounding = tabRounding;
    style.TabBorderSize = tabBorderSize;
    style.ButtonTextAlign = buttonTextAlign;
    style.SelectableTextAlign = selectableTextAlign;
    style.Alpha = alpha;
    style.DisabledAlpha = disabledAlpha;
    
    // Force ImGui to refresh all windows with the new style
    // This ensures changes are immediately visible across all editor windows
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void StyleConfiguration::ResetToDefaults() {
    ImGui::StyleColorsDark(); 
    LoadFromImGui();
}

nlohmann::json StyleConfiguration::ToJson() {
    nlohmann::json json;
    
    // Serialize colors
    nlohmann::json colorsJson;
    for (const auto& colorPair : colors) {
        std::string colorKey = std::to_string(static_cast<int>(colorPair.first));
        const ImVec4& color = colorPair.second;
        colorsJson[colorKey] = {color.x, color.y, color.z, color.w};
    }
    json["colors"] = colorsJson;
    
    // Serialize sizes and spacing
    json["windowPadding"] = {windowPadding.x, windowPadding.y};
    json["windowRounding"] = windowRounding;
    json["windowBorderSize"] = windowBorderSize;
    json["framePadding"] = {framePadding.x, framePadding.y};
    json["frameRounding"] = frameRounding;
    json["frameBorderSize"] = frameBorderSize;
    json["itemSpacing"] = {itemSpacing.x, itemSpacing.y};
    json["itemInnerSpacing"] = {itemInnerSpacing.x, itemInnerSpacing.y};
    json["indentSpacing"] = indentSpacing;
    json["scrollbarSize"] = scrollbarSize;
    json["scrollbarRounding"] = scrollbarRounding;
    json["grabMinSize"] = grabMinSize;
    json["grabRounding"] = grabRounding;
    json["tabRounding"] = tabRounding;
    json["tabBorderSize"] = tabBorderSize;
    json["buttonTextAlign"] = {buttonTextAlign.x, buttonTextAlign.y};
    json["selectableTextAlign"] = {selectableTextAlign.x, selectableTextAlign.y};
    json["alpha"] = alpha;
    json["disabledAlpha"] = disabledAlpha;
    
    return json;
}

void StyleConfiguration::FromJson(const nlohmann::json& json) {
    int validColorsLoaded = 0;
    int invalidColorsSkipped = 0;
    
    try {
        // Deserialize colors with enhanced error handling
        if (json.contains("colors") && json["colors"].is_object()) {
            std::map<ImGuiCol_, ImVec4> tempColors;
            
            for (const auto& colorItem : json["colors"].items()) {
                try {
                    int colorIndex = std::stoi(colorItem.key());
                    if (colorIndex >= 0 && colorIndex < ImGuiCol_COUNT) {
                        const auto& colorArray = colorItem.value();
                        if (colorArray.is_array() && colorArray.size() >= 4) {
                            ImVec4 color;
                            
                            // Safely extract color values with type checking
                            if (colorArray[0].is_number() && colorArray[1].is_number() && 
                                colorArray[2].is_number() && colorArray[3].is_number()) {
                                
                                color.x = colorArray[0].get<float>();
                                color.y = colorArray[1].get<float>();
                                color.z = colorArray[2].get<float>();
                                color.w = colorArray[3].get<float>();
                                
                                // Clamp color values to valid range instead of rejecting
                                color.x = std::clamp(color.x, 0.0f, 1.0f);
                                color.y = std::clamp(color.y, 0.0f, 1.0f);
                                color.z = std::clamp(color.z, 0.0f, 1.0f);
                                color.w = std::clamp(color.w, 0.0f, 1.0f);
                                
                                tempColors[static_cast<ImGuiCol_>(colorIndex)] = color;
                                validColorsLoaded++;
                            } else {
                                invalidColorsSkipped++;
                            }
                        } else {
                            invalidColorsSkipped++;
                        }
                    } else {
                        invalidColorsSkipped++;
                    }
                } catch (const std::exception&) {
                    invalidColorsSkipped++;
                    continue;
                }
            }
            
            // Only update colors if we successfully loaded some
            if (validColorsLoaded > 0) {
                for (const auto& colorPair : tempColors) {
                    colors[colorPair.first] = colorPair.second;
                }
            }
        }
    } catch (const std::exception&) {
        invalidColorsSkipped++;
    }
    
    // Deserialize sizes and spacing with validation
    try {
        if (json.contains("windowPadding") && json["windowPadding"].is_array() && json["windowPadding"].size() >= 2) {
            float x = json["windowPadding"][0].get<float>();
            float y = json["windowPadding"][1].get<float>();
            if (x >= 0.0f && y >= 0.0f) {
                windowPadding.x = x;
                windowPadding.y = y;
            }
        }
        if (json.contains("windowRounding") && json["windowRounding"].is_number()) {
            float value = json["windowRounding"].get<float>();
            if (value >= 0.0f) windowRounding = value;
        }
        if (json.contains("windowBorderSize") && json["windowBorderSize"].is_number()) {
            float value = json["windowBorderSize"].get<float>();
            if (value >= 0.0f) windowBorderSize = value;
        }
        if (json.contains("framePadding") && json["framePadding"].is_array() && json["framePadding"].size() >= 2) {
            float x = json["framePadding"][0].get<float>();
            float y = json["framePadding"][1].get<float>();
            if (x >= 0.0f && y >= 0.0f) {
                framePadding.x = x;
                framePadding.y = y;
            }
        }
        if (json.contains("frameRounding") && json["frameRounding"].is_number()) {
            float value = json["frameRounding"].get<float>();
            if (value >= 0.0f) frameRounding = value;
        }
        if (json.contains("frameBorderSize") && json["frameBorderSize"].is_number()) {
            float value = json["frameBorderSize"].get<float>();
            if (value >= 0.0f) frameBorderSize = value;
        }
        if (json.contains("itemSpacing") && json["itemSpacing"].is_array() && json["itemSpacing"].size() >= 2) {
            float x = json["itemSpacing"][0].get<float>();
            float y = json["itemSpacing"][1].get<float>();
            if (x >= 0.0f && y >= 0.0f) {
                itemSpacing.x = x;
                itemSpacing.y = y;
            }
        }
        if (json.contains("itemInnerSpacing") && json["itemInnerSpacing"].is_array() && json["itemInnerSpacing"].size() >= 2) {
            float x = json["itemInnerSpacing"][0].get<float>();
            float y = json["itemInnerSpacing"][1].get<float>();
            if (x >= 0.0f && y >= 0.0f) {
                itemInnerSpacing.x = x;
                itemInnerSpacing.y = y;
            }
        }
        if (json.contains("indentSpacing") && json["indentSpacing"].is_number()) {
            float value = json["indentSpacing"].get<float>();
            if (value >= 0.0f) indentSpacing = value;
        }
        if (json.contains("scrollbarSize") && json["scrollbarSize"].is_number()) {
            float value = json["scrollbarSize"].get<float>();
            if (value >= 0.0f) scrollbarSize = value;
        }
        if (json.contains("scrollbarRounding") && json["scrollbarRounding"].is_number()) {
            float value = json["scrollbarRounding"].get<float>();
            if (value >= 0.0f) scrollbarRounding = value;
        }
        if (json.contains("grabMinSize") && json["grabMinSize"].is_number()) {
            float value = json["grabMinSize"].get<float>();
            if (value >= 0.0f) grabMinSize = value;
        }
        if (json.contains("grabRounding") && json["grabRounding"].is_number()) {
            float value = json["grabRounding"].get<float>();
            if (value >= 0.0f) grabRounding = value;
        }
        if (json.contains("tabRounding") && json["tabRounding"].is_number()) {
            float value = json["tabRounding"].get<float>();
            if (value >= 0.0f) tabRounding = value;
        }
        if (json.contains("tabBorderSize") && json["tabBorderSize"].is_number()) {
            float value = json["tabBorderSize"].get<float>();
            if (value >= 0.0f) tabBorderSize = value;
        }
        if (json.contains("buttonTextAlign") && json["buttonTextAlign"].is_array() && json["buttonTextAlign"].size() >= 2) {
            float x = json["buttonTextAlign"][0].get<float>();
            float y = json["buttonTextAlign"][1].get<float>();
            if (x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f) {
                buttonTextAlign.x = x;
                buttonTextAlign.y = y;
            }
        }
        if (json.contains("selectableTextAlign") && json["selectableTextAlign"].is_array() && json["selectableTextAlign"].size() >= 2) {
            float x = json["selectableTextAlign"][0].get<float>();
            float y = json["selectableTextAlign"][1].get<float>();
            if (x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f) {
                selectableTextAlign.x = x;
                selectableTextAlign.y = y;
            }
        }
        if (json.contains("alpha") && json["alpha"].is_number()) {
            float value = json["alpha"].get<float>();
            if (value >= 0.0f && value <= 1.0f) alpha = value;
        }
        if (json.contains("disabledAlpha") && json["disabledAlpha"].is_number()) {
            float value = json["disabledAlpha"].get<float>();
            if (value >= 0.0f && value <= 1.0f) disabledAlpha = value;
        }
    } catch (const std::exception&) {
        // If any individual property fails to deserialize, skip it and continue
        // The object will retain its default or previously loaded values
    }
}

void StyleConfiguration::SaveToFile(const std::string& path) {
    try {
        nlohmann::json json = ToJson();
        std::filesystem::path filePath(path);
        
        // Ensure the directory exists
        std::filesystem::create_directories(filePath.parent_path());
        
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << json.dump(4);
            file.close();
        } else {
            Logger::LogError("Failed to open file for writing: " + path);
        }
    } catch (const std::exception& e) {
        Logger::LogError("Failed to save style configuration: " + std::string(e.what()));
    }
}

void StyleConfiguration::LoadFromFile(const std::string& path) {
    try {
        std::filesystem::path filePath(path);
        
        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            throw std::runtime_error("Configuration file does not exist: " + path);
        }
        
        // Read JSON from file
        std::ifstream file(filePath);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;
            file.close();
            
            // Load configuration from JSON
            FromJson(json);
        } else {
            Logger::LogError("Failed to open file for reading: " + path);
        }
    } catch (const std::exception& e) {
        Logger::LogError("Failed to load style configuration: " + std::string(e.what()));
    }
}