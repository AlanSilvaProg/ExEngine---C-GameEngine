#include <SDL2/SDL_image.h>
#include "AssetManager.h"
#include "../../Logger/Logger.h"
#include "../Rendering/Renderer/ExRendererGetters.h"

AssetManager::AssetManager(){
    if(TTF_Init() != 0){
        Logger::LogError("TTF Initialization error with the follow message: " + std::string(TTF_GetError()));
    }
};

AssetManager::~AssetManager(){
    Release();
};

SDL_Texture* AssetManager::GetTextureAsset(std::string id, std::string path)
{
    auto textureFinded = textureMap.find(id);
    if(textureFinded != textureMap.end()){
        return textureFinded->second;
    }

    SDL_Surface* surface = IMG_Load(path.c_str());

    if(surface == nullptr){
        Logger::LogError("Fail to load Image at path : " + path + " \n With the follow message: " + IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ExRendererGetters::renderer, surface);
    SDL_FreeSurface(surface);

    textureMap[id] = texture;
    return texture;
};

SDL_Texture* AssetManager::GetTextureAsset(std::string id, std::string path, TTF_Font* font)
{
    return NULL;
};

void AssetManager::Release(){
    if(TTF_WasInit() > 0)
    {
        TTF_Quit();
    }

    for(auto keyPair : textureMap){
        SDL_DestroyTexture(keyPair.second);
    }

    textureMap.clear();
};