#include "ExRenderer.h"
#include "../../../Logger/Logger.h"
#include "../../Settings/EngineSettings.h"
#include "../../Utils/Color.h"

SDL_Renderer* ExRenderer::renderer;
SDL_Window* ExRenderer::window;
bool ExRenderer::initialized;

void ExRenderer::Initialize(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        Logger::LogError("SDL initialization error with the message: " + std::string(SDL_GetError()));
        return;
    }

    window = window = SDL_CreateWindow(EngineSettings::GetEngineStringId().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_BORDERLESS);

    if(!window)
    {
        Logger::LogError("SDL Window creation error with the message: " + std::string(SDL_GetError()));
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    if(!renderer)
    {
        Logger::LogError("SDL Renderer creation error with the message: " + std::string(SDL_GetError()));
        return;
    }
    
    SDL_PumpEvents();
    initialized = true;
};

void ExRenderer::RenderSequence(){
    if(!initialized) return;
    PreRender();
    Render();
    PostRender();
};

void ExRenderer::PreRender(){
    //cleaning window with a base color 
    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
};

void ExRenderer::Render(){
};

void ExRenderer::PostRender(){
    //Post effects + window render
};

SDL_Renderer* ExRenderer::GetRenderer(){
    return renderer;
};

void ExRenderer::Quit(){
    if(!initialized) return;

    SDL_Quit();
};