#pragma once
#include <string>
#include "../../Serializer/ExEngineSerializer.h"
#include "../../Serializer/ISerializable.h"
#include "../../Serializer/ExSerializedClass.h"

struct LayerAttributes : public ISerializable{
private:
    std::string _layerId;
public:
    int layerIndex;
    int layerOrderIndex;

    std::string layerId() const { return _layerId; };

    virtual ExSerializedClass Serialize() override{
        return ExSerializedClass{
            Demangle(typeid(*this).name()),
            {
                EX_SERIALIZER((*this), _layerId, false),
                EX_SERIALIZER((*this), layerIndex, true),
                EX_SERIALIZER((*this), layerOrderIndex, true)
            }
        };
    };
};