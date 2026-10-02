#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>
#include <memory>
#include "../../Resource/IReleasable.h"
#include "TextureAssetReference.h"
#include "FontAssetReference.h"

class AssetManager : public IReleasable{
private:
    static std::shared_ptr<AssetManager> instance;

    std::map<std::string, TextureAssetReference*> textureMap;
    std::map<std::string, FontAssetReference*> fontMap;
protected:
    void Release() override;
public:
    inline static const std::string DEFAULT_FONT_ID = "default-font";

    static std::shared_ptr<AssetManager> GetInstance();

    AssetManager();
    ~AssetManager();

    SDL_Texture* GetTexture(const std::string& id, const std::string& path);
    TTF_Font* GetFont(const std::string& id, const std::string& path);
    void FreeAsset(std::string id);
};