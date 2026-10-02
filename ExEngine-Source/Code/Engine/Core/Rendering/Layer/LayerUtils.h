#pragma once
#include <memory>
#include "LayerAttributes.h"
#include "../../ECS/ECSManager.h"
#include "../../Components/Rendering/SpriteComponent.h"
#include "../../Components/UI/TextLabelComponent.h"

namespace LayerUtils{

    // Every renderable component type that carries its own LayerAttributes - checked in order
    // when walking up the hierarchy looking for an ancestor's layer to inherit.
    inline const LayerAttributes* FindOwnLayerAttributes(const std::shared_ptr<EntityCS>& entity){
        if(auto sprite = entity->GetComponent<SpriteComponent>()) return &sprite->layerAttributes;
        if(auto label = entity->GetComponent<TextLabelComponent>()) return &label->layerAttributes;
        return nullptr;
    };

    // `ownLayerAttributes` wins outright if it explicitly overrides; otherwise walks up the parent
    // chain for the nearest ancestor that has a renderable component with its own explicit layer,
    // skipping ancestors with no renderable component (or that are themselves just inheriting).
    // Falls back to `ownLayerAttributes` if no ancestor ever sets one.
    inline const LayerAttributes& ResolveLayerAttributes(const std::shared_ptr<EntityCS>& entity, const LayerAttributes& ownLayerAttributes){
        if(ownLayerAttributes.overrideLayer) return ownLayerAttributes;

        auto current = entity->GetParent();
        while(current != nullptr){
            if(const auto* ancestorLayer = FindOwnLayerAttributes(current)){
                if(ancestorLayer->overrideLayer) return *ancestorLayer;
            }
            current = current->GetParent();
        }

        return ownLayerAttributes;
    };

};
