#include <SDL2/SDL_image.h>
#include "AssetManager.h"
#include "../../Logger/Logger.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include <new>

std::shared_ptr<AssetManager> AssetManager::instance = nullptr;

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
        return textureFinded->second->GetNewReference();
    }

    auto assetReference = new AssetReference(path);
    textureMap[id] =  assetReference;

    return assetReference->GetNewReference();
};

SDL_Texture* AssetManager::GetTextureAsset(std::string id, std::string path, TTF_Font* font)
{
    return nullptr;
};

void AssetManager::FreeAsset(std::string id){
    textureMap[id]->ReleaseReference();
};

void AssetManager::Release(){
    if(TTF_WasInit() > 0)
    {
        TTF_Quit();
    }

    for(auto keyPair : textureMap){
        keyPair.second->FreeAllResources();
        delete(keyPair.second);
    }

    textureMap.clear();
};

std::shared_ptr<AssetManager> AssetManager::GetInstance(){
    if(instance == nullptr){
        instance = std::make_shared<AssetManager>();
    }

    return instance;
};