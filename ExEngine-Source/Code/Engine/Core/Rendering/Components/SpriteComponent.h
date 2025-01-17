#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <memory>
#include "../../AssetManager/AssetManager.h"
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"

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
};