#pragma once
#include "../../../ECS/ECSManager.h"
#include "../../../Rendering/Layer/LayerAttributes.h"
#include <SDL2/SDL.h>
#include <memory>

struct IRenderElement{
public:
    const LayerAttributes& layerAttributes;

    IRenderElement(const LayerAttributes& layerAttributes) : layerAttributes(layerAttributes) {};

    virtual void Render(SDL_Renderer* renderer) = 0;
    virtual const LayerAttributes& GetLayerAttributes() const { return layerAttributes; };
};