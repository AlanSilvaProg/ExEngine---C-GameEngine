#include "../../Components/UI/TextLabelComponent.h"
#include "../../Components/Core/TransformComponent.h"
#include "../../Rendering/Renderer/ExRendererGetters.h"
#include "../../Rendering/Renderer/RenderQueue/FontRenderElement.h"
#include "../../Utils/Anchor/AnchorUtils.h"
#include "../../Utils/Transform/TransformUtils.h"
#include "../../Rendering/Layer/LayerUtils.h"
#include "TextLabelSystem.h"
#include <algorithm>

TextLabelSystem::TextLabelSystem(){
    Require<TransformComponent>(false);
    Require<TextLabelComponent>(false);
};

void TextLabelSystem::UpdateSystem(SystemContext systemContext){
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
    glm::vec2 cameraPosition(cameraTransformComponent->position.x, cameraTransformComponent->position.y);

    for(auto entity : entities){
        const auto& transformComponent = entity->GetComponent<TransformComponent>();
        const auto& textLabelComponent = entity->GetComponent<TextLabelComponent>();

        const auto texture = textLabelComponent->texture;
        const auto textureRect = textLabelComponent->textureRect;

        if(texture == nullptr || textureRect == nullptr) continue;

        glm::vec2 worldPosition = AnchorUtils::GetPivotAdjustedPosition(entity, transformComponent);
        glm::vec2 screenPosition = ExRendererGetters::WorldToScreen(worldPosition, cameraPosition);
        auto worldTransform = TransformUtils::GetWorldTransform(entity);

        double angle = worldTransform.rotation;

        // The texture is rendered once at a fixed, high-resolution font size (MAX_FONT_SIZE, see
        // FontAssetReference) and scaled down here to the requested display `size`, so changing
        // `size` never needs the glyphs to be re-rendered.
        float sizeScale = static_cast<float>(textLabelComponent->size) / static_cast<float>(MAX_FONT_SIZE);
        float naturalWidth = textureRect->w * sizeScale * worldTransform.scale.x * cameraZoom;
        float naturalHeight = textureRect->h * sizeScale * worldTransform.scale.y * cameraZoom;

        SDL_Rect dstRect;

        if(textLabelComponent->fit){
            const auto& exRect = textLabelComponent->exRect;
            glm::vec2 localMin = glm::min(exRect.beginRect, exRect.endRect);
            glm::vec2 localMax = glm::max(exRect.beginRect, exRect.endRect);

            glm::vec2 boxScreenMin = ExRendererGetters::WorldToScreen(worldPosition + localMin, cameraPosition);
            glm::vec2 boxScreenMax = ExRendererGetters::WorldToScreen(worldPosition + localMax, cameraPosition);

            float boxWidth = boxScreenMax.x - boxScreenMin.x;
            float boxHeight = boxScreenMax.y - boxScreenMin.y;

            // Only ever shrinks to fit - text already smaller than the box keeps its natural size.
            float fitScale = 1.0f;
            if(naturalWidth > 0.0f && naturalHeight > 0.0f && boxWidth > 0.0f && boxHeight > 0.0f){
                fitScale = std::min(1.0f, std::min(boxWidth / naturalWidth, boxHeight / naturalHeight));
            }

            dstRect = {
                static_cast<int>(boxScreenMin.x),
                static_cast<int>(boxScreenMin.y),
                static_cast<int>(naturalWidth * fitScale),
                static_cast<int>(naturalHeight * fitScale)
            };
        }
        else{
            dstRect = {
                static_cast<int>(screenPosition.x),
                static_cast<int>(screenPosition.y),
                static_cast<int>(naturalWidth),
                static_cast<int>(naturalHeight)
            };
        }

        auto renderElementPtr = std::make_shared<FontRenderElement>(texture, textureRect, dstRect, angle, LayerUtils::ResolveLayerAttributes(entity, textLabelComponent->layerAttributes));
        ExRendererGetters::AddToRenderQueue(renderElementPtr);
    }
};
