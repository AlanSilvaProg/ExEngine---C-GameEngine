#include "ExRenderer.h"
#include "../../../Logger/Logger.h"
#include "../../Settings/EngineSettings.h"
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
    renderingSystem2D = ecsManager->CreateSystem<RenderingSystem2D>();
    cameraSystem = ecsManager->CreateSystem<CameraSystem>(renderingSystem2D);
    initialized = true;

    PreRenderEventHandler::Create();
};

void ExRenderer::RenderSequence(){
    if(!initialized) return;
    
    PreRenderEventHandler::preRenderHandler->Invoke();

    cameraSystem->UpdateSystem();

    PreRenderEventHandler::postRenderHandler->Invoke();
};

std::shared_ptr<RenderingSystem2D> ExRenderer::GetRenderingSystem2D(){
    return renderingSystem2D;
};

void ExRenderer::Quit(){
    if(!initialized) return;

    SDL_Quit();
};