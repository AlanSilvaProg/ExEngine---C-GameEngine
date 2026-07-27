#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <memory>
#include <filesystem>
#include "../../SpecialFields/SpriteReferenceField/SpriteReference.h"
#include "../../AssetManager/AssetManager.h"
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"

struct SpriteComponent : public EComponentS<SpriteComponent>{
private:
    std::shared_ptr<AssetManager> assetManager;
    std::string loadedTextureId;

    inline void GetTextureInformation(){
        SDL_Point point;
        SDL_QueryTexture(texture, NULL, NULL, &point.x, &point.y);
        this->srcRect = new SDL_Rect { 0, 0, point.x, point.y };
    };

public:
    SpriteReference spriteReference;
    LayerAttributes layerAttributes;
    SDL_Texture* texture;
    SDL_Rect* srcRect;
    bool flipX;
    bool flipY;

    SpriteComponent(){
        texture = nullptr;
        srcRect = nullptr;
        assetManager = AssetManager::GetInstance();
    };

    SpriteComponent(std::string id, std::filesystem::path path, int layerIndex, int layerOrderIndex, bool flipX, bool flipY) : flipX(flipX), flipY(flipY)
    {
        layerAttributes.layerIndex = layerIndex;
        layerAttributes.layerOrderIndex = layerOrderIndex;

        assetManager = AssetManager::GetInstance();
        SetSprite(id, path);
    };

    ~SpriteComponent() {
        assetManager->FreeAsset(loadedTextureId);
    };

    inline SpriteComponent& SetSprite(std::string spriteId, std::filesystem::path spritePath){
        if(!loadedTextureId.empty() && loadedTextureId != spriteId)
            assetManager->FreeAsset(loadedTextureId);

        spriteReference.id = spriteId;
        spriteReference.path = spritePath;
        texture = assetManager->GetTextureAsset(spriteReference.id, spriteReference.path);
        loadedTextureId = spriteId;
        GetTextureInformation();

        return *this;
    };

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER_CB((*this), spriteReference, true, ([this](){ SetSprite(spriteReference.id, spriteReference.path); })),
                EX_SERIALIZER((*this), layerAttributes, true)
            }
        };
    };

    virtual nlohmann::json ToJson() override {
        return {
            {"spriteReference", spriteReference.ToJson()},
            {"layerAttributes", layerAttributes.ToJson()},
            {"flipX", flipX},
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("spriteReference"))
            spriteReference.FromJson(json["spriteReference"]);

        if (json.contains("layerAttributes"))
            layerAttributes.FromJson(json["layerAttributes"]);

        if (json.contains("flipX"))
            flipX = json["flipX"];

        SetSprite(spriteReference.id, spriteReference.path);
    };
};

//ToDo update sprite by editor options
REGISTER_COMPONENT(SpriteComponent)