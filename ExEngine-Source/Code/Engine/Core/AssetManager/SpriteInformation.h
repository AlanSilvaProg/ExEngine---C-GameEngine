#pragma once
#include "../../Logger/Logger.h"
#include "AssetManager.h"
#include <filesystem>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <SDL.h>

struct SpriteInformation{
private:
    std::filesystem::path spritePath;
    glm::vec2 spriteDivision;
    glm::vec2 initialPoint;
    glm::vec2 finalPoint;
    glm::vec2 spriteSize;
    glm::vec2 spriteSizeNormalized;

    std::shared_ptr<AssetManager> assetManager;
public:
    std::string spriteId;
    SDL_Texture* texture;

    inline SpriteInformation(const std::string id, const std::filesystem::path path, glm::vec2 division) : spriteId(id), spritePath(path), spriteDivision(division) {};
    inline ~SpriteInformation() { AssetManager::GetInstance()->FreeAsset(spriteId); };

    inline SDL_Texture* GetTexture() 
    {
        if(texture == nullptr) 
            return CacheContent();
        return texture; 
    };

    inline const glm::vec2 GetSpriteSize() 
    {
        if(texture == nullptr) 
            CacheContent();
        return { spriteSize.x, spriteSize.y };
    }

    inline const glm::vec2 GetInitialRect(const glm::vec2& spritePosition) {
        if(texture == nullptr) 
            CacheContent();

        float x,y;

        if(spritePosition.x >= spriteDivision.x) 
            x = spriteSizeNormalized.x * (spriteDivision.x -1);
        else
            x = spriteSizeNormalized.x * spritePosition.x;

        if(spritePosition.y >= spriteDivision.y) 
            y = spriteSizeNormalized.y * (spriteDivision.y -1);
        else
            y = spriteSizeNormalized.y * spritePosition.y;

        return {x, y};
    };

    inline const glm::vec2 GetFinalRect(const glm::vec2& spritePosition){
        if(texture == nullptr) 
            CacheContent();

        float x, y;
        if(spritePosition.x >= spriteDivision.x -1) 
            x = 1;
        else
            x = spriteSizeNormalized.x * (spritePosition.x + 1);

        if(spritePosition.y >= spriteDivision.y -1) 
            y = 1;
        else
            y = spriteSizeNormalized.y * (spritePosition.y + 1);

            return {x, y};
    };

    inline SDL_Texture* CacheContent() 
    {
        if(texture == nullptr)
            texture = AssetManager::GetInstance()->GetTextureAsset(spriteId, spritePath);
        int w = 0,h = 0;
        SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
        spriteSize.x = (static_cast<float>(w)/spriteDivision.x);
        spriteSizeNormalized.x = spriteSize.x/static_cast<float>(w);
        spriteSize.y = (static_cast<float>(h)/spriteDivision.y);
        spriteSizeNormalized.y = spriteSize.y/static_cast<float>(h);
        return texture; 
    };
};