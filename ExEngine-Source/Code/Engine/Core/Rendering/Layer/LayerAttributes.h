#pragma once
#include <string>
#include "../../Serializer/ExEngineSerializer.h"
#include "../../Serializer/ISerializable.h"
#include "../../Serializer/ExSerializedClass.h"
#include "../../../JsonUtility/IJsonConvertable.h"

struct LayerAttributes : public ISerializable, public IJsonConvertable{
private:
    std::string _layerId;
public:
    int layerIndex = 0;
    int layerOrderIndex = 0;

    // false (default) means "nothing explicitly set here" - the nearest ancestor's own
    // LayerAttributes (if any) is used instead; see LayerUtils::ResolveLayerAttributes. Flipping
    // this on makes layerIndex/layerOrderIndex win regardless of what any parent has.
    bool overrideLayer = false;

    std::string layerId() const { return _layerId; };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER((*this), _layerId, false),
        EX_SERIALIZER((*this), layerIndex, true),
        EX_SERIALIZER((*this), layerOrderIndex, true),
        EX_SERIALIZER((*this), overrideLayer, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"_layerId", _layerId},
            {"layerIndex", layerIndex},
            {"layerOrderIndex", layerOrderIndex},
            {"overrideLayer", overrideLayer}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("_layerId"))
            _layerId = json["_layerId"];

        if (json.contains("layerIndex"))
            layerIndex = json["layerIndex"];

        if (json.contains("layerOrderIndex"))
            layerOrderIndex = json["layerOrderIndex"];

        if (json.contains("overrideLayer"))
            overrideLayer = json["overrideLayer"].get<bool>();
    }
};