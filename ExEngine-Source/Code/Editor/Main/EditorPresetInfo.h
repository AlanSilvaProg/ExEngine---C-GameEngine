#pragma once
#include "../../Engine/JsonUtility/IJsonConvertable.h"
#include <string>

struct EditorPresetInfo : public IJsonConvertable{
public:
    bool sceneViewEnabled;
    bool gameViewEnabled;
    bool projectSettingsEnabled;
    bool assetBrowserIsOpened;
    std::string editorLayout;

    EditorPresetInfo() = default;
    ~EditorPresetInfo() = default;

    virtual nlohmann::json ToJson() override {
        return {
            {"sceneViewEnabled", sceneViewEnabled},
            {"gameViewEnabled", gameViewEnabled},
            {"projectSettingsEnabled", projectSettingsEnabled},
            {"assetBrowserIsOpened", assetBrowserIsOpened},
            {"editorLayout", editorLayout}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("sceneViewEnabled")) sceneViewEnabled = json["sceneViewEnabled"];
        if (json.contains("gameViewEnabled")) gameViewEnabled = json["gameViewEnabled"];
        if (json.contains("projectSettingsEnabled")) projectSettingsEnabled = json["projectSettingsEnabled"];
        if (json.contains("assetBrowserIsOpened")) assetBrowserIsOpened = json["assetBrowserIsOpened"];
        if (json.contains("editorLayout")) editorLayout = json["editorLayout"];
    }
};