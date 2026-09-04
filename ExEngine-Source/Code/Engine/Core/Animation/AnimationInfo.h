#pragma once
#include "AnimationStep.h"
#include "../AssetManager/AssetManager.h"
#include "../../JsonUtility/IJsonConvertable.h"
#include "../Serializer/ExEngineSerializer.h"
#include "../Serializer/ISerializable.h"
#include "../Serializer/ExSerializedClass.h"
#include <memory>
#include <vector>
#include <string>

enum AnimationLoopType{
    NONE,
    LOOP,
    PINGPONG
};

struct AnimationInfo : public ISerializable, public IJsonConvertable{
private:
    bool running;
    int lastCalledIndex = -1;
public:
    AnimationLoopType animationLoopType;
    std::vector<AnimationStep> animationSteps;

    inline AnimationStep* GetCurrentAnimationStep(float currentTime){
        if(animationSteps.empty()) return nullptr;

        int currentIndex = -1;

        for(int i = 0; i < static_cast<int>(animationSteps.size()); ++i){
            if(animationSteps[i].secondsToTrigger > currentTime) break;
            currentIndex = i;
        }

        if(lastCalledIndex != -1 && lastCalledIndex != currentIndex){
            animationSteps[lastCalledIndex].ResetStep();
        }

        lastCalledIndex = currentIndex;
        return currentIndex != -1 ? &animationSteps[currentIndex] : nullptr;
    };

    inline float GetAnimationDurationInSecs(){
        if(animationSteps.size() == 0) return 0;

        return animationSteps.back().secondsToTrigger;
    };

    inline bool IsRunning(){
        return running;
    };

    inline void SetIsRunning(){
        running = true;        
    };

    inline void Stop(){
        running = false;
        if(lastCalledIndex != -1){
            animationSteps[lastCalledIndex].ResetStep();
        }
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), running, false),
        EX_SERIALIZER((*this), animationLoopType, true)
    )

    inline virtual nlohmann::json ToJson() override{
        nlohmann::json stepsJson = nlohmann::json::array();
        for (auto& step : animationSteps) {
            stepsJson.push_back(step.ToJson());
        }

        return {
            {"animationLoopType", animationLoopType},
            {"animationSteps", stepsJson}
        };
    };

    inline virtual void FromJson(const nlohmann::json& json) override{
        if (json.contains("animationLoopType")) animationLoopType = json["animationLoopType"];

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