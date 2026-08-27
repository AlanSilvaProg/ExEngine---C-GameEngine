#pragma once
#include "../AnimationInfo.h"
#include "../../Rendering/Components/SpriteComponent.h"
#include "../../ECS/Component/EComponentS.h"
#include "../../ECS/ECSManager.h"
#include "../../SpecialFields/TimeUpdateKindField/TimeUpdateKind.h"

struct AnimationComponent : public EComponentS<AnimationComponent>{
public:
    static constexpr unsigned int ComponentId = 4;

    TimeUpdateKind timeUpdateKind;
    AnimationInfo animationInfo;

    inline void Evaluate(){
        //add progress basead on time

        RefreshCurrentState();
    };

    inline void EvaluateTo(float targetTimeInSec){
        //jump to a specific time of this animation

        RefreshCurrentState();
    };

    inline void RefreshCurrentState(){
        //update values based on current step
    };

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER((*this), timeUpdateKind, true),
                EX_SERIALIZER((*this), animationInfo, true)
            }
        };
    };

    virtual nlohmann::json ToJson() override {
        return {
            {"timeUpdateKind", timeUpdateKind},
            {"animationInfo", animationInfo.ToJson()}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("timeUpdateKind")) timeUpdateKind = json["timeUpdateKind"];
        if (json.contains("animationInfo")) animationInfo.FromJson(json["animationInfo"]);
    };
};

REGISTER_COMPONENT(AnimationComponent)