#pragma once
#include <SDL2/SDL.h>

class ExRenderer{
private:
    static SDL_Renderer* renderer;
    static SDL_Window* window;
    static bool initialized;

    static void PreRender();
    static void Render();
    static void PostRender();
public:
    static void Initialize();

    static void RenderSequence();

    static void Quit();

    static SDL_Renderer* GetRenderer();
};