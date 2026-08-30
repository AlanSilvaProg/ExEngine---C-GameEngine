#pragma once
#include "../../JsonUtility/IJsonConvertable.h"
#include "EntityContent.h"
#include <nlohmann/json.hpp>

class AnimationStep : public IJsonConvertable{
private:
    EntityContent stepContent;
    bool returned;
public:
    float secondsToTrigger;

    inline void SetNewSetStateContent(const EntityContent& content) { stepContent = content; };
    inline EntityContent& GetStepStateContent(){
        returned = true;
        return stepContent;
    };

    inline const bool WasReturned() const { return returned; };
    inline void ResetStep() { returned = false; };

    inline virtual nlohmann::json ToJson() override{
        return {
            {"stepContent", stepContent.ToJson()},
            {"secondsToTrigger", secondsToTrigger}
        };
    };

    inline virtual void FromJson(const nlohmann::json& json) override{
        if (json.contains("stepContent")) stepContent.FromJson(json["stepContent"]);
        if (json.contains("secondsToTrigger")) secondsToTrigger = json["secondsToTrigger"].get<float>();
    };
};