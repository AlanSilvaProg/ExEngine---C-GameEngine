#pragma once
#include "../../JsonUtility/IJsonConvertable.h"
#include "../Runtime/Settings/RuntimeSettings.h"
#include "nlohmann/json.hpp"
#include <string>

struct ConfigurationFileProperties : public IJsonConvertable{
public:
    int targetFramesPerSeconds;
    std::string externalTextEditorPath;
    bool physicsEnabled;

    virtual nlohmann::json ToJson() override {
        //Target FPS
        targetFramesPerSeconds = RuntimeSettings::GetTargetFps();
        //External text/script editor
        externalTextEditorPath = RuntimeSettings::GetExternalTextEditorPath();
        //Physics engine
        physicsEnabled = RuntimeSettings::GetPhysicsEnabled();
        return nlohmann::json{
            {"targetFramesPerSeconds", targetFramesPerSeconds},
            {"externalTextEditorPath", externalTextEditorPath},
            {"physicsEnabled", physicsEnabled}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        //Target FPS
        json.at("targetFramesPerSeconds").get_to(targetFramesPerSeconds);
        RuntimeSettings::SetTargetFps(targetFramesPerSeconds);

        //External text/script editor - optional for backwards compatibility with older config files
        externalTextEditorPath = json.value("externalTextEditorPath", std::string(""));
        RuntimeSettings::SetExternalTextEditorPath(externalTextEditorPath);

        //Physics engine - optional for backwards compatibility with older config files
        physicsEnabled = json.value("physicsEnabled", true);
        RuntimeSettings::SetPhysicsEnabled(physicsEnabled);
    }
};