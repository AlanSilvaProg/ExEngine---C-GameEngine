#pragma once
#include <string>
#include <SDL2/SDL_ttf.h>

#ifndef MAX_FONT_SIZE
#define MAX_FONT_SIZE 128
#endif

class FontAssetReference{
public:
    TTF_Font* font;

    std::string path;
    int refCount;

    FontAssetReference(std::string path) : font(nullptr), path(path), refCount(0){};
    ~FontAssetReference();

    TTF_Font* GetNewReference();
    void ReleaseReference();
    void FreeAllResources();
};