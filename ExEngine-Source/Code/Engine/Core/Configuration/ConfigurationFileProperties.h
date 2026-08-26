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
    bool drawAllPhysics;
    bool autoSaveOnPlay;

    virtual nlohmann::json ToJson() override {
        //Target FPS
        targetFramesPerSeconds = RuntimeSettings::GetTargetFps();
        //External text/script editor
        externalTextEditorPath = RuntimeSettings::GetExternalTextEditorPath();
        //Physics engine
        physicsEnabled = RuntimeSettings::GetPhysicsEnabled();
        //Draw all physics colliders in the editor Scene View
        drawAllPhysics = RuntimeSettings::GetDrawAllPhysics();
        //Auto-save the world when pressing Play
        autoSaveOnPlay = RuntimeSettings::GetAutoSaveOnPlay();
        return nlohmann::json{
            {"targetFramesPerSeconds", targetFramesPerSeconds},
            {"externalTextEditorPath", externalTextEditorPath},
            {"physicsEnabled", physicsEnabled},
            {"drawAllPhysics", drawAllPhysics},
            {"autoSaveOnPlay", autoSaveOnPlay}
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

        //Draw all physics colliders - optional for backwards compatibility with older config files
        drawAllPhysics = json.value("drawAllPhysics", true);
        RuntimeSettings::SetDrawAllPhysics(drawAllPhysics);

        //Auto-save on Play - optional for backwards compatibility with older config files
        autoSaveOnPlay = json.value("autoSaveOnPlay", false);
        RuntimeSettings::SetAutoSaveOnPlay(autoSaveOnPlay);
    }
};