#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "../../Components/CameraComponent.h"

class ExRendererGetters{
public:
    static SDL_Renderer* renderer;
    static SDL_Window* window;
    static std::shared_ptr<EntityCS> currentRenderCamera;

    // Resolution management
    static int renderWidth;
    static int renderHeight;
    
    // Resolution methods
    static void SetRenderResolution(int width, int height);
    static void GetRenderResolution(int& width, int& height);
};