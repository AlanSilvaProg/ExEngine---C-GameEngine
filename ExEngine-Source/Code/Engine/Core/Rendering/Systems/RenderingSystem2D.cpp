#include "RenderingSystem2D.h"
#include "../Renderer/ExRendererGetters.h"
#include "../../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../../../Logger/Logger.h"
#include <algorithm>
#include <SDL2/SDL.h>

RenderingSystem2D::RenderingSystem2D(){
    Require<TransformComponent>();    
    Require<SpriteComponent>();

    assetManager = AssetManager::GetInstance();
    //Create Optional value and use for layer  component maybe ?
};

void RenderingSystem2D::UpdateSystem() {
    if(systemEntities.size() == 0)
        return;

    std::sort(systemEntities.begin(), systemEntities.end(), [this](const EntityCS& a, const EntityCS& b) { 
              return this->RenderOrderCheck(a, b); 
          });

    for(auto entity : systemEntities){
        auto spriteComponent = entity.GetComponent<SpriteComponent>();
        auto transformComponent = entity.GetComponent<TransformComponent>();

        auto texture = spriteComponent->texture;

        transformComponent->position.x += 0.1;

        //render texture
        SDL_Rect dstRect = {
            static_cast<int>(transformComponent->position.x),
            static_cast<int>(transformComponent->position.y),
            static_cast<int>(spriteComponent->srcRect->w * transformComponent->scale.x),
            static_cast<int>(spriteComponent->srcRect->h * transformComponent->scale.y)
        };
    
        SDL_RenderCopy(ExRendererGetters::renderer, texture, spriteComponent->srcRect, &dstRect);
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