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

        SDL_Point point;
        SDL_QueryTexture(texture, NULL, NULL, &point.x, &point.y);
        this->srcRect = new SDL_Rect { 0, 0, point.x, point.y };
    };

    ~SpriteComponent() {
        assetManager->FreeAsset(id);
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
};

//ToDo update sprite by editor options
REGISTER_COMPONENT(SpriteComponent)