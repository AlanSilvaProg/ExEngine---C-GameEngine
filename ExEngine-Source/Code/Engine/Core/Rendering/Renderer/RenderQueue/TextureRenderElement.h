#pragma once
#include "IRenderElement.h"

struct TextureRenderElement : public IRenderElement{
public:
    SDL_Texture* texture; 
    SDL_Rect* srcRect;
    SDL_Rect dstRect;
    double angle;
    SDL_Point* center;
    SDL_RendererFlip flip;

    TextureRenderElement(SDL_Texture* texture, SDL_Rect* srcRect, SDL_Rect dstRect, double angle, SDL_Point* center, SDL_RendererFlip flip, const LayerAttributes& layerAttributes) : texture(texture), srcRect(srcRect), dstRect(dstRect), angle(angle), center(center), flip(flip), IRenderElement(layerAttributes) {};

    virtual void Render(SDL_Renderer* renderer) override {
        SDL_RenderCopyEx(renderer, texture, srcRect, &dstRect, angle, NULL, flip);
    };
};