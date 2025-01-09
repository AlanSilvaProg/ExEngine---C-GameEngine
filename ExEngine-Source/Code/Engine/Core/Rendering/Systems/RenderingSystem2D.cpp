#include "RenderingSystem2D.h"
#include "../Renderer/ExRendererGetters.h"
#include "../../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <algorithm>

RenderingSystem2D::RenderingSystem2D(){
    Require<TransformComponent>();    
    Require<SpriteComponent>();
    //Create Optional value
};

void RenderingSystem2D::UpdateSystem() {
    if(systemEntities.size() == 0)
        return;

    std::sort(systemEntities.begin(), systemEntities.end(), [this](const EntityCS& a, const EntityCS& b) { 
              return this->RenderOrderCheck(a, b); 
          });

    
};

bool RenderingSystem2D::RenderOrderCheck(const EntityCS& a, const EntityCS& b){
    auto aSpriteComponent = a.GetComponent<SpriteComponent>();
    auto bSpriteComponent = b.GetComponent<SpriteComponent>();
    auto aLayer = aSpriteComponent->layerAttributes;
    auto bLayer = bSpriteComponent->layerAttributes;

    if(aLayer.layerIndex == bLayer.layerIndex)
    {
        return aLayer.layerOrderIndex <= bLayer.layerOrderIndex;
    }

    return aLayer.layerIndex <= bLayer.layerOrderIndex;
};