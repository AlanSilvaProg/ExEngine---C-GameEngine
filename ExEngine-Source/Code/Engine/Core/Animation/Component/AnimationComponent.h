#pragma once
#include "../AnimationInfo.h"
#include "../../Rendering/Components/SpriteComponent.h"
#include "../../ECS/Component/EComponentS.h"
#include "../../ECS/ECSManager.h"
#include "../../Runtime/Time/Time.h"
#include <vector>

struct AnimationComponent : public EComponentS<AnimationComponent>{
public:
    static constexpr unsigned int ComponentId = 4;

    float currentTime;
    AnimationInfo animationInfo;

    AnimationComponent() = default;
    AnimationComponent(const AnimationComponent& animationComponent){
        currentTime = animationComponent.currentTime;
        animationInfo = animationComponent.animationInfo;
    };
    ~AnimationComponent() = default;

    //add progress basead on time
    inline AnimationStep* Evaluate(const SystemContext context){
        auto timePassed = context == SystemContext::FIXED_UPDATE ? Time::fixedDeltaTime : Time::deltaTime;

        return EvaluateTo(currentTime + timePassed);
    };

    //jump to a specific time of this animation
    inline AnimationStep* EvaluateTo(const float targetTimeInSec){
        auto totalDuration = animationInfo.GetAnimationDurationInSecs(); // ToDo this refresh is only needed within editor, I need to make it cached for game builds

        if(currentTime > totalDuration){
            switch (animationInfo.animationLoopType)
            {
            case AnimationLoopType::NONE:
                currentTime = totalDuration;
                Stop();
                break;
            
            default:
                currentTime = 0; // ToDo implement ping pong
                break;
            }
        }

        currentTime = targetTimeInSec > totalDuration ? totalDuration : targetTimeInSec;

        return animationInfo.GetCurrentAnimationStep(currentTime);
    };

    inline AnimationStep* Start(){
        if(!animationInfo.IsRunning()){
            animationInfo.SetIsRunning();
            return animationInfo.GetCurrentAnimationStep(currentTime);
        }
    };

    inline AnimationStep* Restart(){
        return EvaluateTo(0);
    };

    inline AnimationStep* RestartAndPlay(){
        currentTime = 0;
        animationInfo.SetIsRunning();
        return animationInfo.GetCurrentAnimationStep(currentTime);
    };

    inline void Stop(){
        animationInfo.Stop();
    };

    inline bool IsRunning(){
        return animationInfo.IsRunning();
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), currentTime, true),
        EX_SERIALIZER((*this), animationInfo, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"currentTime", currentTime},
            {"animationInfo", animationInfo.ToJson()}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("currentTime")) currentTime = json["currentTime"].get<float>();
        if (json.contains("animationInfo")) animationInfo.FromJson(json["animationInfo"]);
    };
};

REGISTER_COMPONENT(AnimationComponent)