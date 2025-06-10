#include "ExRenderer.h"
#include "../../../Logger/Logger.h"
#include "../../Settings/EngineSettings.h"
#include "../../Utils/Color.h"
#include "ExRendererGetters.h"
#include "RendererEvent/PreRenderEventHandler.h"

bool ExRenderer::initialized = false;
std::shared_ptr<ECSManager> ExRenderer::ecsManager = nullptr;
std::shared_ptr<RenderingSystem2D> ExRenderer::renderingSystem2D = nullptr;
std::shared_ptr<CameraSystem> ExRenderer::cameraSystem = nullptr;

void ExRenderer::Initialize(std::shared_ptr<ECSManager> ecsManagerPtr){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        Logger::LogError("SDL initialization error with the message: " + std::string(SDL_GetError()));
        return;
    }

    ExRendererGetters::window = ExRendererGetters::window = SDL_CreateWindow(EngineSettings::GetEngineStringId().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

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
    cameraSystem = ecsManager->CreateSystem<CameraSystem>();
    renderingSystem2D = ecsManager->CreateSystem<RenderingSystem2D>(cameraSystem);
    initialized = true;

    UpdateDisplayTexture(0);
    PreRenderEventHandler::Create();
};

void ExRenderer::RenderSequence(){
    if(!initialized) return;
    Render();
};

void ExRenderer::PreRender(){
    UpdateDisplayTexture(0);

    cameraSystem->UpdateSystem();

    //cleaning window with a base color 
    auto color = Color::BLUE;
    SDL_SetRenderDrawColor(ExRendererGetters::renderer, color->r, color->g, color->b, color->a);
    SDL_RenderClear(ExRendererGetters::renderer);
    
    renderingSystem2D->UpdateSystem();
    
    PreRenderEventHandler::preRenderHandler->Invoke();
};

void ExRenderer::Render(){
    PreRender();
    PostRender();
};

void ExRenderer::PostRender(){
    //Post effects + window render

    PreRenderEventHandler::postRenderHandler->Invoke();
    
    DisableDisplayTextures(0);
    SDL_RenderPresent(ExRendererGetters::renderer);

    PreRenderEventHandler::postRenderPresentHandler->Invoke();
};

void ExRenderer::UpdateDisplayTexture(int displayIndex){
#ifdef EXENGINE_EDITOR

    if(ExRendererGetters::sceneDisplay.find(displayIndex) == ExRendererGetters::sceneDisplay.end())
    {
        auto sdlTexture = SDL_CreateTexture(ExRendererGetters::renderer, SDL_PIXELFORMAT_ABGR1555, SDL_TEXTUREACCESS_TARGET, 800, 800);
        ExRendererGetters::sceneDisplay.emplace(displayIndex, sdlTexture);
        
        SDL_SetRenderTarget(ExRendererGetters::renderer, sdlTexture);
        return;
    }
    
    SDL_SetRenderTarget(ExRendererGetters::renderer, ExRendererGetters::sceneDisplay[displayIndex]);

#endif
};

void ExRenderer::DisableDisplayTextures(int displayIndex){
#ifdef EXENGINE_EDITOR

    if(ExRendererGetters::sceneDisplay.find(displayIndex) == ExRendererGetters::sceneDisplay.end()) return;

    SDL_SetRenderTarget(ExRendererGetters::renderer, NULL);

#endif
};

void ExRenderer::Quit(){
    if(!initialized) return;

    SDL_Quit();
};