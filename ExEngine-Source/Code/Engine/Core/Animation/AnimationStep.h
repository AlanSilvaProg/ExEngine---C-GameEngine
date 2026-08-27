#pragma once
#include "../SpecialFields/SpriteReferenceField/SpriteReference.h"
#include "../Rendering/Layer/LayerAttributes.h"
#include "../ECS/InternalRegistry/ComponentRegistry.h"
#include "../Utils/Algorithms/JsonExtensions.h"
#include "../../JsonUtility/IJsonConvertable.h"
#include <SDL.h>

class AnimationStep : public IJsonConvertable{
    SDL_Texture* texture;
    SDL_Rect* srcRect;

public:
    float durantionInSecs; //by rule, it rounds over every 0,5 unit ( can't be 0.2 )
    SpriteReference spriteReference;
    LayerAttributes layerAttributes;

    virtual nlohmann::json ToJson() override{
        return {
            {"durantionInSecs", durantionInSecs},
            {"spriteReference", spriteReference.ToJson()},
            {"layerAttributes", layerAttributes.ToJson()}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override{
        if (json.contains("durantionInSecs")) durantionInSecs = json["durantionInSecs"].get<float>();
        if (json.contains("spriteReference")) spriteReference.FromJson(json["spriteReference"]);
        if (json.contains("layerAttributes")) layerAttributes.FromJson(json["layerAttributes"]);
    };
};