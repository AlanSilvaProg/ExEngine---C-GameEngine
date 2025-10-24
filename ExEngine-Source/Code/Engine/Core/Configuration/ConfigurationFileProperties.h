#pragma once
#include "../../JsonUtility/IJsonConvertable.h"
#include "../Runtime/Settings/RuntimeSettings.h"
#include "nlohmann/json.hpp"

struct ConfigurationFileProperties : public IJsonConvertable{
public:
    int targetFramesPerSeconds;

    virtual nlohmann::json ToJson() override {
        //Target FPS
        targetFramesPerSeconds = RuntimeSettings::GetTargetFps();
        return nlohmann::json{{"targetFramesPerSeconds", targetFramesPerSeconds}};
    }

    virtual void FromJson(const nlohmann::json& json) override {
        //Target FPS
        json.at("targetFramesPerSeconds").get_to(targetFramesPerSeconds);
        RuntimeSettings::SetTargetFps(targetFramesPerSeconds);
    }
};