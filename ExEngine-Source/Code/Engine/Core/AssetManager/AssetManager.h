#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>
#include <memory>
#include "../../Resource/IReleasable.h"
#include "AssetReference.h"

class AssetManager : public IReleasable{
private:
    static std::shared_ptr<AssetManager> instance;

    std::map<std::string, AssetReference*> textureMap;
protected:
    void Release() override;
public:
    static std::shared_ptr<AssetManager> GetInstance();

    AssetManager();
    ~AssetManager();

    SDL_Texture* GetTextureAsset(std::string id, std::string path);
    SDL_Texture* GetTextureAsset(std::string id, std::string path, TTF_Font* font);
    void FreeAsset(std::string id);
};