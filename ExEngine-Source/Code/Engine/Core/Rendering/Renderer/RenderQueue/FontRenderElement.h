#pragma once
#include "IRenderElement.h"

struct FontRenderElement : public IRenderElement{
public:
    SDL_Texture* texture;
    SDL_Rect* srcRect;
    SDL_Rect dstRect;
    double angle;

    FontRenderElement(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect dstRect, double angle, const LayerAttributes& layerAttributes)
        : texture(texture), srcRect(srcRect), dstRect(dstRect), angle(angle), IRenderElement(layerAttributes) {};

    virtual void Render(SDL_Renderer* renderer) override {
        SDL_RenderCopyEx(renderer, texture, srcRect, &dstRect, angle, nullptr, SDL_FLIP_NONE);
    };
};
