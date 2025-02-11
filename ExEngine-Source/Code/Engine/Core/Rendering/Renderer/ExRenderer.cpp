#include "ExRenderer.h"
#include "../../../Logger/Logger.h"
#include "../../Settings/EngineSettings.h"
#include "../../Utils/Color.h"
#include "ExRendererGetters.h"
#include "RendererEvent/PreRenderEventHandler.h"

bool ExRenderer::initialized = false;
std::shared_ptr<ECSManager> ExRenderer::ecsManager = nullptr;
std::shared_ptr<RenderingSystem2D> ExRenderer::renderingSystem2D = nullptr;

void ExRenderer::Initialize(std::shared_ptr<ECSManager> ecsManagerPtr){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        Logger::LogError("SDL initialization error with the message: " + std::string(SDL_GetError()));
        return;
    }

    ExRendererGetters::window = ExRendererGetters::window = SDL_CreateWindow(EngineSettings::GetEngineStringId().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_FOREIGN);

    if(!ExRendererGetters::window)
    {
        Logger::LogError("SDL Window creation error with the message: " + std::string(SDL_GetError()));
        return;
    }

    ExRendererGetters::renderer = SDL_CreateRenderer(ExRendererGetters::window, -1, 0);

    if(!ExRendererGetters::renderer)
    {
        Logger::LogError("SDL Renderer creation error with the message: " + std::string(SDL_GetError()));
        return;
    }
    
    SDL_PumpEvents();

    ecsManager = ecsManagerPtr;
    renderingSystem2D = ecsManager->CreateSystem<RenderingSystem2D>();
    initialized = true;

    PreRenderEventHandler::Create();
};

void ExRenderer::RenderSequence(){
    if(!initialized) return;
    Render();
};

void ExRenderer::PreRender(){
    //cleaning window with a base color 
    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(ExRendererGetters::renderer);
    
    renderingSystem2D->UpdateSystem();
    
    PreRenderEventHandler::handler->Invoke(0);
};

void ExRenderer::Render(){
    PreRender();
    PostRender();
};

void ExRenderer::PostRender(){
    //Post effects + window render

    SDL_RenderPresent(ExRendererGetters::renderer);
};

void ExRenderer::Quit(){
    if(!initialized) return;

    SDL_Quit();
};