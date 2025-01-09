#pragma once
#include <string>

struct LayerAttributes {
private:
    std::string _layerId;
public:
    int layerIndex;
    int layerOrderIndex;

    std::string layerId() const { return _layerId; };
};