#pragma once
#include "Windows/EditorWindows/BuildSettings/BuildType.h"
#include "../../Engine/JsonUtility/IJsonConvertable.h"
#include <string>

struct EditorPresetInfo : public IJsonConvertable{
public:
    bool inspectorEnabled;
    bool entityBrowserEnabled;
    bool projectSettingsEnabled;
    bool consoleEnabled;
    bool ecsMonitoringEnabled;
    bool ecsAdministratorEnabled;
    bool assetBrowserIsOpened;
    bool engineConfigEnabled;
    bool animationEditorEnabled;
    std::string currentWorldPath;
    BuildType buildTarget;
    std::string editorLayout;

    EditorPresetInfo() = default;
    ~EditorPresetInfo() = default;

    virtual nlohmann::json ToJson() override {
        return {
            {"inspectorEnabled", inspectorEnabled},
            {"entityBrowserEnabled", entityBrowserEnabled},
            {"projectSettingsEnabled", projectSettingsEnabled},
            {"consoleEnabled", consoleEnabled},
            {"ecsMonitoringEnabled", ecsMonitoringEnabled},
            {"ecsAdministratorEnabled", ecsAdministratorEnabled},
            {"assetBrowserIsOpened", assetBrowserIsOpened},
            {"engineConfigEnabled", engineConfigEnabled},
            {"animationEditorEnabled", animationEditorEnabled},
            {"currentWorldPath", currentWorldPath},
            {"buildTarget", buildTarget},
            {"editorLayout", editorLayout}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("inspectorEnabled")) inspectorEnabled = json["inspectorEnabled"];
        if (json.contains("entityBrowserEnabled")) entityBrowserEnabled = json["entityBrowserEnabled"];
        if (json.contains("projectSettingsEnabled")) projectSettingsEnabled = json["projectSettingsEnabled"];
        if (json.contains("consoleEnabled")) consoleEnabled = json["consoleEnabled"];
        if (json.contains("ecsMonitoringEnabled")) ecsMonitoringEnabled = json["ecsMonitoringEnabled"];
        if (json.contains("ecsAdministratorEnabled")) ecsAdministratorEnabled = json["ecsAdministratorEnabled"];
        if (json.contains("assetBrowserIsOpened")) assetBrowserIsOpened = json["assetBrowserIsOpened"];
        if (json.contains("engineConfigEnabled")) engineConfigEnabled = json["engineConfigEnabled"];
        if (json.contains("animationEditorEnabled")) animationEditorEnabled = json["animationEditorEnabled"];
        if (json.contains("currentWorldPath")) currentWorldPath = json["currentWorldPath"];
        if (json.contains("buildTarget")) buildTarget = json["buildTarget"];
        if (json.contains("editorLayout")) editorLayout = json["editorLayout"];
    };
};