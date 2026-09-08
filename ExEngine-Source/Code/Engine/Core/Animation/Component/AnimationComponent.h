#pragma once
#include "../AnimationInfo.h"
#include "../../Rendering/Components/SpriteComponent.h"
#include "../../ECS/Component/EComponentS.h"
#include "../../ECS/ECSManager.h"
#include "../../Runtime/Time/Time.h"
#include <vector>

struct AnimationComponent : public EComponentS<AnimationComponent>{
private: 
    bool startAutomatically;
    unsigned int currentAnimation;
public:
    static constexpr unsigned int ComponentId = 4;

    float currentTime;
    std::vector<AnimationInfo> animationInfo = { AnimationInfo{} }; // always has at least one entry

    AnimationComponent() = default;
    AnimationComponent(const AnimationComponent& animationComponent){
        currentTime = animationComponent.currentTime;
        animationInfo = animationComponent.animationInfo;
    };
    ~AnimationComponent() = default;

    inline const bool ValidateAutoPlay(){
        if(!startAutomatically) return false;
        startAutomatically = false;
        return true;
    };

    inline const unsigned int CreateNewAnimation(){
        animationInfo.emplace_back();
        return animationInfo.size() -1;
    };

    inline void ChangeAnimation(const unsigned int animationIndex){
        ResetCurrentAnimation();
        currentAnimation = animationIndex;
        Start();
    };

    //add progress basead on time
    inline AnimationStep* Evaluate(const SystemContext context){
        auto timePassed = context == SystemContext::FIXED_UPDATE ? Time::fixedDeltaTime : Time::deltaTime;

        return EvaluateTo(currentTime + timePassed);
    };

    //jump to a specific time of this animation
    inline AnimationStep* EvaluateTo(const float targetTimeInSec){
        auto& currentAnimationInfo = animationInfo[currentAnimation];
        auto totalDuration = currentAnimationInfo.GetAnimationDurationInSecs(); // ToDo this refresh is only needed within editor, I need to make it cached for game builds

        if(currentTime > totalDuration){
            switch (currentAnimationInfo.animationLoopType)
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

        return currentAnimationInfo.GetCurrentAnimationStep(currentTime);
    };

    inline AnimationStep* Start(){
        if(!animationInfo[currentAnimation].IsRunning()){
            animationInfo[currentAnimation].SetIsRunning();
            return animationInfo[currentAnimation].GetCurrentAnimationStep(currentTime);
        }
    };

    inline AnimationStep* Restart(){
        return EvaluateTo(0);
    };

    inline AnimationStep* RestartAndPlay(){
        currentTime = 0;
        animationInfo[currentAnimation].SetIsRunning();
        return animationInfo[currentAnimation].GetCurrentAnimationStep(currentTime);
    };

    inline void ResetCurrentAnimation(){
        currentTime = 0;
        animationInfo[currentAnimation].Stop();
    };

    inline void Stop(){
        animationInfo[currentAnimation].Stop();
    };

    inline bool IsRunning(){
        return animationInfo[currentAnimation].IsRunning();
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), startAutomatically, true),
        EX_SERIALIZER((*this), currentTime, true),
        EX_SERIALIZER((*this), animationInfo, true)
    )

    virtual nlohmann::json ToJson() override {
        nlohmann::json animationInfoJson = nlohmann::json::array();
        for (auto& info : animationInfo) {
            animationInfoJson.push_back(info.ToJson());
        }

        return {
            {"startAutomatically", startAutomatically},
            {"currentTime", currentTime},
            {"animationInfo", animationInfoJson}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("startAutomatically")) startAutomatically = json["startAutomatically"].get<bool>();
        if (json.contains("currentTime")) currentTime = json["currentTime"].get<float>();

        if (json.contains("animationInfo")) {
            animationInfo.clear();
            const auto& animationInfoJson = json["animationInfo"];

            if (animationInfoJson.is_object()) {
                // Legacy save format: animationInfo used to be a single object before
                // becoming a list. Iterating a json object with a range-based for yields
                // its member values one by one, not the object itself, so it must be
                // handled separately rather than falling into the array branch below.
                AnimationInfo info;
                info.FromJson(animationInfoJson);
                animationInfo.push_back(info);
            }
            else {
                for (const auto& infoJson : animationInfoJson) {
                    AnimationInfo info;
                    info.FromJson(infoJson);
                    animationInfo.push_back(info);
                }
            }
        }
    };
};

REGISTER_COMPONENT(AnimationComponent)