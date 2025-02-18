#pragma once
#include <SDL2/SDL.h>
#include <map>

class ExRendererGetters{
public:
    static SDL_Renderer* renderer;
    static SDL_Window* window;

    static std::map<int, SDL_Texture*> sceneDisplay;
};