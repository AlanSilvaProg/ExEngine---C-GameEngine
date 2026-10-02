#pragma once
#include <SDL2/SDL.h>
#include <string>

class TextureAssetReference{
public:
    SDL_Texture* texture;

    std::string path;
    int refCount;

    TextureAssetReference(std::string path) : texture(nullptr), path(path), refCount(0){};
    ~TextureAssetReference();

    SDL_Texture* GetNewReference();
    void ReleaseReference();
    void FreeAllResources();
};