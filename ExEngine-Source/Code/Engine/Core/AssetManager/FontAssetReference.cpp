#include "FontAssetReference.h"
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

FontAssetReference::~FontAssetReference(){
    FreeAllResources();
};

TTF_Font* FontAssetReference::GetNewReference(){
    if(font == nullptr){
        auto resolvedPath = ResolveAssetPath(path);
        font = TTF_OpenFont(resolvedPath.c_str(), MAX_FONT_SIZE);

        if(font == nullptr){
            Logger::LogError("Fail to load Image at path : " + resolvedPath.string() + " \n With the follow message: " + TTF_GetError());
            return NULL;
        }

        refCount = 0;
    }
    refCount += 1;
    return font;
};

void FontAssetReference::ReleaseReference(){
    if(refCount == 0 || font == nullptr) return;

    refCount -= 1;

    if(refCount == 0)
    {
        FreeAllResources();
    }
};

void FontAssetReference::FreeAllResources(){
    if(font == nullptr) return;

    TTF_CloseFont(font);
    font = nullptr;
}