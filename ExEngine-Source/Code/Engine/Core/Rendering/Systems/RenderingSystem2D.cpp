#include "RenderingSystem2D.h"
#include "../Renderer/ExRendererGetters.h"
#include "../../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../../../Logger/Logger.h"
#include <algorithm>
#include <SDL2/SDL.h>

RenderingSystem2D::RenderingSystem2D(){
    Require<TransformComponent>(false);    
    Require<SpriteComponent>(false);

    assetManager = AssetManager::GetInstance();
};

void RenderingSystem2D::UpdateSystem() {
    if(systemEntities.size() == 0)
        return;

    std::sort(systemEntities.begin(), systemEntities.end(), [this](const EntityCS& a, const EntityCS& b) { 
              return this->RenderOrderCheck(a, b); 
          });

    auto currentCamera = ExRendererGetters::currentRenderCamera;

    if(currentCamera == nullptr)
    {
        Logger::LogWarning("No camera available to render!");
    }

    for(auto entity : systemEntities){
        auto spriteComponent = entity.GetComponent<SpriteComponent>();
        auto transformComponent = entity.GetComponent<TransformComponent>();

        auto cameraTransformComponent = currentCamera->GetComponent<TransformComponent>();

        auto texture = spriteComponent->texture;

        //ToDo draw a white rect or similiar by default
        if(texture == nullptr) return;

        //render texture
        SDL_Rect dstRect = {
            static_cast<int>(transformComponent->position.x - cameraTransformComponent->position.x),
            static_cast<int>(transformComponent->position.y - cameraTransformComponent->position.y),
            static_cast<int>(spriteComponent->srcRect->w * transformComponent->scale.x),
            static_cast<int>(spriteComponent->srcRect->h * transformComponent->scale.y)
        };

        double angle = transformComponent->rotation.x;
        int flip = SDL_FLIP_NONE;

        if(spriteComponent->flipX)
        {
            flip |= SDL_FLIP_HORIZONTAL;
        }

        if(spriteComponent->flipY)
        {
            flip |= SDL_FLIP_VERTICAL;
        }

        SDL_RenderCopyEx(ExRendererGetters::renderer, texture, spriteComponent->srcRect, &dstRect, angle, NULL, static_cast<SDL_RendererFlip>(flip));
    }
};

bool RenderingSystem2D::RenderOrderCheck(const EntityCS& a, const EntityCS& b){
    auto aSpriteComponent = a.GetComponent<SpriteComponent>();
    auto bSpriteComponent = b.GetComponent<SpriteComponent>();
    
    auto aLayer = aSpriteComponent->layerAttributes;
    auto bLayer = bSpriteComponent->layerAttributes;

    if(aLayer.layerIndex == bLayer.layerIndex)
    {
        return aLayer.layerOrderIndex < bLayer.layerOrderIndex;
    }

    return aLayer.layerIndex < bLayer.layerOrderIndex;
};