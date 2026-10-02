#include "TextureAssetReference.h"
#include "../Engine.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../../Logger/Logger.h"
#include <SDL2/SDL_image.h>
#include <string>
#include <filesystem>

namespace{
    std::filesystem::path ResolveAssetPath(const std::string& storedPath){
        std::filesystem::path path = storedPath;

        if(path.empty())
            return path;

        if(!path.is_absolute())
            return Engine::GetGameAssetsPath() / path;

        if(std::filesystem::exists(path))
            return path;

        std::string pathStr = path.generic_string();
        const std::string marker = "/Assets/";
        auto assetsPos = pathStr.rfind(marker);
        if(assetsPos != std::string::npos)
            return Engine::GetGameAssetsPath() / pathStr.substr(assetsPos + marker.size());

        return path;
    }
}

TextureAssetReference::~TextureAssetReference(){
    FreeAllResources();
};

SDL_Texture* TextureAssetReference::GetNewReference(){
    if(texture == nullptr){
        auto resolvedPath = ResolveAssetPath(path);
        auto surface = IMG_Load(resolvedPath.string().c_str());

        if(surface == nullptr){
            Logger::LogError("Fail to load Image at path : " + resolvedPath.string() + " \n With the follow message: " + IMG_GetError());
            return NULL;
        }

        texture = SDL_CreateTextureFromSurface(ExRendererGetters::renderer, surface);
        SDL_FreeSurface(surface);
        refCount = 0;
    }
    refCount += 1;
    return texture;
};

void TextureAssetReference::ReleaseReference(){
    if(refCount == 0 || texture == nullptr) return;

    refCount -= 1;

    if(refCount == 0)
    {
        FreeAllResources();
    }
};

void TextureAssetReference::FreeAllResources(){
    if(texture == nullptr) return;

    SDL_DestroyTexture(texture);
    texture = nullptr;
}