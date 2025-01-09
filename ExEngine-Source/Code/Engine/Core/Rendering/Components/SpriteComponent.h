#pragma once
#include <string>
#include "../../ECS/ECSManager.h"
#include "../Layer/LayerAttributes.h"

struct SpriteComponent : public EComponentS<SpriteComponent>{
public:
    std::string id;
    std::string path;
    LayerAttributes layerAttributes;

    SpriteComponent(std::string id, std::string path, int layerIndex, int layerOrderIndex) : id(id), path(path)
    {
        layerAttributes.layerIndex = layerIndex;
        layerAttributes.layerOrderIndex = layerOrderIndex;
    };

    ~SpriteComponent() = default;
};