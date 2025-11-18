#pragma once 
#include <SDL2/SDL.h>
#include <string>

class AssetReference{
public:
    SDL_Texture* texture;
    
    std::string path;
    int refCount;

    AssetReference(std::string path) : path(path){};
    ~AssetReference();

    SDL_Texture* GetNewReference();
    void ReleaseReference();
    void FreeAllResources();
};