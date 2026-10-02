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

SDL_Texture* AssetManager::GetTexture(const std::string& id, const std::string& path)
{
    auto textureFinded = textureMap.find(id);

    if(textureFinded != textureMap.end()){
        return textureFinded->second->GetNewReference();
    }

    auto textureReference = new TextureAssetReference(path);
    auto receivedTextureReference = textureReference->GetNewReference();

    if(receivedTextureReference == nullptr){
        delete(textureReference);
        return nullptr;
    }

    textureMap[id] =  textureReference;

    return receivedTextureReference;
};


TTF_Font* AssetManager::GetFont(const std::string& id, const std::string& path)
{
    auto fontFinded = fontMap.find(id);

    if(fontFinded != fontMap.end()){
        return fontFinded->second->GetNewReference();
    }

    auto fontReference = new FontAssetReference(path);
    auto receivedNewReference = fontReference->GetNewReference();

    if(receivedNewReference == nullptr){
        delete(fontReference);
        return nullptr;
    }

    fontMap[id] =  fontReference;

    return receivedNewReference;
};

void AssetManager::FreeAsset(std::string id){
    if(textureMap[id])
        textureMap[id]->ReleaseReference();

    if(fontMap[id])
        fontMap[id]->ReleaseReference();
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