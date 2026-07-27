#pragma once 
#include <SDL2/SDL.h>
#include <string>

class AssetReference{
public:
    SDL_Texture* texture;
    
    std::string path;
    int refCount;

    AssetReference(std::string path) : texture(nullptr), path(path), refCount(0){};
    ~AssetReference();

    SDL_Texture* GetNewReference();
    void ReleaseReference();
    void FreeAllResources();
};