#include <SDL2/SDL_image.h>
#include "AssetManager.h"
#include "../../Logger/Logger.h"
#include "../Rendering/Renderer/ExRenderer.h"

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
    if(textureMap.find(id) != textureMap.end()){
        return textureMap[id];
    }

    SDL_Surface* surface = IMG_Load(path.c_str());
    SDL_Texture* texture = SDL_CreateTextureFromSurface(ExRenderer::GetRenderer(), surface);
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