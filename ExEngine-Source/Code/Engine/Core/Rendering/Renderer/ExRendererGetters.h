#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <memory>
#include "../../Components/CameraComponent.h"

class ExRendererGetters{
public:
    static SDL_Renderer* renderer;
    static SDL_Window* window;

    static std::map<int, SDL_Texture*> sceneDisplay;
};