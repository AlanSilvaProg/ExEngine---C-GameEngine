#include "ExRenderer.h"
#include "../../../Logger/Logger.h"
#include "../../Settings/EngineSettings.h"
#include "ExRendererGetters.h"
#include "RendererEvent/PreRenderEventHandler.h"

bool ExRenderer::initialized = false;
std::shared_ptr<ECSManager> ExRenderer::ecsManager = nullptr;
std::shared_ptr<RenderingSystem2D> ExRenderer::renderingSystem2D = nullptr;
std::shared_ptr<TextLabelSystem> ExRenderer::textLabelSystem = nullptr;
std::shared_ptr<ECSystemContext> ExRenderer::preRenderSystemContext = nullptr;

void ExRenderer::Initialize(std::shared_ptr<ECSManager> ecsManagerPtr){
#ifdef __EMSCRIPTEN__
    // Emscripten's SDL2 port has no haptic (force-feedback) implementation - requesting it makes
    // SDL_Init() fail outright for every subsystem, not just skip that one.
    const Uint32 sdlInitFlags = SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC;
#else
    const Uint32 sdlInitFlags = SDL_INIT_EVERYTHING;
#endif

    if(SDL_Init(sdlInitFlags) != 0){
        Logger::LogError("SDL initialization error with the message: " + std::string(SDL_GetError()));
        return;
    }

    // A normal resizable/maximized window - WorldToScreen/ScreenToWorld read the actual drawable
    // size each call (see ExRendererGetters.cpp), so the camera stays centered regardless of
    // whatever size the window ends up being.
    ExRendererGetters::window = SDL_CreateWindow(EngineSettings::GetEngineStringId().c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

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
    preRenderSystemContext = ecsManager->GetECSystemContext(SystemContext::PRE_RENDER);

    //camera system creation and context registry
    renderingSystem2D = ecsManager->CreateSystem<RenderingSystem2D>();
    textLabelSystem = ecsManager->CreateSystem<TextLabelSystem>();
    auto cameraSystem = ecsManager->CreateSystem<CameraSystem>(renderingSystem2D, textLabelSystem);
    auto cameraSystemTypeId = std::type_index(typeid(CameraSystem));
    preRenderSystemContext->Register(cameraSystemTypeId, cameraSystem);

    initialized = true;
};

void ExRenderer::RenderSequence(){
    if(!initialized) return;
    
    //CameraSystem (registered in the PRE_RENDER context) drives renderings per active camera
    PreRenderEventHandler::preRenderHandler->Invoke();
    RenderQueue();
    PreRenderEventHandler::postRenderHandler->Invoke();
};

void ExRenderer::RenderQueue(){
    auto& currentRenderQueue = ExRendererGetters::currentRenderQueue;

    std::sort(currentRenderQueue.begin(), currentRenderQueue.end(), [](const std::shared_ptr<IRenderElement> a, const std::shared_ptr<IRenderElement> b) { 
            return ExRenderer::RenderOrderCheck(a->GetLayerAttributes(), b->GetLayerAttributes()); 
        });

    for(auto& renderElement : currentRenderQueue){
        renderElement->Render(ExRendererGetters::renderer);
    }

    currentRenderQueue.clear();
};

const bool ExRenderer::RenderOrderCheck(const LayerAttributes& a, const LayerAttributes& b){
    const auto& aLayer = a;
    const auto& bLayer = b;

    if(aLayer.layerIndex == bLayer.layerIndex)
    {
        return aLayer.layerOrderIndex < bLayer.layerOrderIndex;
    }

    return aLayer.layerIndex < bLayer.layerOrderIndex;
};

std::shared_ptr<RenderingSystem2D> ExRenderer::GetRenderingSystem2D(){
    return renderingSystem2D;
};

std::shared_ptr<TextLabelSystem> ExRenderer::GetTextLabelSystem(){
    return textLabelSystem;
};

void ExRenderer::SetGameplayCameraEnabled(bool enabled){
    if(preRenderSystemContext != nullptr) preRenderSystemContext->enabled = enabled;
};

void ExRenderer::Quit(){
    if(!initialized) return;

    SDL_Quit();
    SDL_DestroyRenderer(ExRendererGetters::renderer);
    SDL_DestroyWindow(ExRendererGetters::window);
};