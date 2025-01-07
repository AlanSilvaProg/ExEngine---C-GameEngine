#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>
#include "../../Resource/IReleasable.h"

class AssetManager : public IReleasable{
private:
    std::map<std::string, SDL_Texture*> textureMap;
protected:
    void Release() override;
public:
    AssetManager();
    ~AssetManager();

    SDL_Texture* GetTextureAsset(std::string id, std::string path);
    SDL_Texture* GetTextureAsset(std::string id, std::string path, TTF_Font* font);
};