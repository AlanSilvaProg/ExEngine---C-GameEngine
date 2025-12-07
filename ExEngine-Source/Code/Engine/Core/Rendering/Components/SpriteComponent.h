#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <memory>
#include "../../AssetManager/AssetManager.h"
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"

struct SpriteComponent : public EComponentS<SpriteComponent>{
private: 
    std::shared_ptr<AssetManager> assetManager;

    inline void GetTextureInformation(){
        SDL_Point point;
        SDL_QueryTexture(texture, NULL, NULL, &point.x, &point.y);
        this->srcRect = new SDL_Rect { 0, 0, point.x, point.y };
    };

public:
    std::string id;
    std::string path;
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

    SpriteComponent(std::string id, std::string path, int layerIndex, int layerOrderIndex, bool flipX, bool flipY) : id(id), path(path), flipX(flipX), flipY(flipY)
    {
        layerAttributes.layerIndex = layerIndex;
        layerAttributes.layerOrderIndex = layerOrderIndex;

        assetManager = AssetManager::GetInstance();
        texture = assetManager->GetTextureAsset(id, path);

        GetTextureInformation();
    };

    ~SpriteComponent() {
        assetManager->FreeAsset(id);
    };

    inline SpriteComponent& SetSprite(std::string spriteId, std::string spritePath){
        id = spriteId;
        path = spritePath;
        texture = assetManager->GetTextureAsset(id, path);
        GetTextureInformation();

        return *this;
    };

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER((*this), id, false),
                EX_SERIALIZER((*this), path, false),
                EX_SERIALIZER((*this), layerAttributes, true)
            }
        };
    };

    virtual nlohmann::json ToJson() override {
        return {
            {"id", id},
            {"path", path},
            {"layerAttributes", layerAttributes.ToJson()},
            {"flipX", flipX},
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        std::string loadedId;
        std::string loadedPath;
        if (json.contains("id"))
            loadedId = json["id"];

        if (json.contains("path"))
            loadedPath = json["path"];

        if (json.contains("layerAttributes"))
            layerAttributes.FromJson(json["layerAttributes"]);

        if (json.contains("flipX"))
            flipX = json["flipX"];

        SetSprite(loadedId, loadedPath);
    };
};

//ToDo update sprite by editor options
REGISTER_COMPONENT(SpriteComponent)