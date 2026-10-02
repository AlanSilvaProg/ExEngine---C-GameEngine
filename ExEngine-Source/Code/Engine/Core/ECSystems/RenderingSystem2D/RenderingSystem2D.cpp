#include "RenderingSystem2D.h"
#include "../../Rendering/Renderer/RenderQueue/TextureRenderElement.h"
#include "../../Rendering/Renderer/ExRendererGetters.h"
#include "../../Components/Core/TransformComponent.h"
#include "../../Components/Rendering/SpriteComponent.h"
#include "../../Utils/Anchor/AnchorUtils.h"
#include "../../../Logger/Logger.h"
#include <algorithm>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

RenderingSystem2D::RenderingSystem2D(){
    Require<TransformComponent>(false);    
    Require<SpriteComponent>(false);

    assetManager = AssetManager::GetInstance();
};

void RenderingSystem2D::UpdateSystem(SystemContext systemContext) {
    if(systemEntities.size() == 0)
        return;

    auto cameraTransformComponent = ExRendererGetters::currentRenderCameraTransform;
    
    if(cameraTransformComponent == nullptr)
    {
        Logger::LogWarning("No camera available to render!");
        return;
    }

    auto entities = systemEntities;

    std::erase_if(entities, [](std::shared_ptr<EntityCS> entity){
        return ExRendererGetters::ShouldOcclude(entity);
    });

    float cameraZoom = ExRendererGetters::globalCameraZoom;

    for(auto entity : entities){
        auto spriteComponent = entity->GetComponent<SpriteComponent>();
        auto transformComponent = entity->GetComponent<TransformComponent>();

        auto texture = spriteComponent->texture;

        //ToDo draw a white rect or similiar by default
        if(texture == nullptr) continue;

        glm::vec2 worldPosition = AnchorUtils::GetPivotAdjustedPosition(entity, transformComponent);

        glm::vec2 cameraPosition(cameraTransformComponent->position.x, cameraTransformComponent->position.y);
        glm::vec2 screenPosition = ExRendererGetters::WorldToScreen(worldPosition, cameraPosition);

        //render texture
        SDL_Rect dstRect = {
            static_cast<int>(screenPosition.x),
            static_cast<int>(screenPosition.y),
            static_cast<int>((spriteComponent->srcRect->w * transformComponent->scale.x) * cameraZoom),
            static_cast<int>((spriteComponent->srcRect->h * transformComponent->scale.y) * cameraZoom)
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

        auto renderElementPtr = std::make_shared<TextureRenderElement>(texture, spriteComponent->srcRect, dstRect, angle, nullptr, static_cast<SDL_RendererFlip>(flip), spriteComponent->layerAttributes);
        ExRendererGetters::AddToRenderQueue(renderElementPtr);
    }
};