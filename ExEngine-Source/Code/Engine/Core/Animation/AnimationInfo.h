#pragma once
#include "AnimationStep.h"
#include "../AssetManager/AssetManager.h"
#include "../Utils/Algorithms/JsonExtensions.h"
#include "../../JsonUtility/IJsonConvertable.h"
#include <memory>
#include <vector>
#include <string>

struct AnimationInfo : public IJsonConvertable{
public:
    std::vector<AnimationStep> animationSteps;

    inline float GetAnimationDurationInSecs(){
        float totalDuration = 0;

        for(const auto& step : animationSteps)
        {
            totalDuration += step.durantionInSecs;
        }

        return totalDuration;
    };

    virtual nlohmann::json ToJson() override{
        nlohmann::json stepsJson = nlohmann::json::array();
        for (auto& step : animationSteps) {
            stepsJson.push_back(step.ToJson());
        }

        return {
            {"animationSteps", stepsJson}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override{
        if (json.contains("animationSteps")) {
            animationSteps.clear();
            for (const auto& stepJson : json["animationSteps"]) {
                AnimationStep step;
                step.FromJson(stepJson);
                animationSteps.push_back(step);
            }
        }
    };
};