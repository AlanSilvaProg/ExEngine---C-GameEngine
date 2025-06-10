#include "AssetReference.h"
#include <SDL2/SDL_image.h>
#include <string>
#include <filesystem> 
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../../Logger/Logger.h"

AssetReference::~AssetReference(){
    FreeAllResources();
};

SDL_Texture* AssetReference::GetNewReference(){
    if(texture == nullptr){ 
        auto surface = IMG_Load(path.c_str());

        if(surface == nullptr){
            Logger::LogError("Fail to load Image at path : " + path + " \n With the follow message: " + IMG_GetError());
            return NULL;
        }

        texture = SDL_CreateTextureFromSurface(ExRendererGetters::renderer, surface);
        SDL_FreeSurface(surface);
        refCount = 0;
    }
    refCount += 1;
    return texture;
};

void AssetReference::ReleaseReference(){
    if(refCount == 0 || texture == nullptr) return;

    refCount -= 1;

    if(refCount == 0)
    {
        FreeAllResources();
    }
};

void AssetReference::FreeAllResources(){
    SDL_DestroyTexture(texture);
    texture = nullptr;
}