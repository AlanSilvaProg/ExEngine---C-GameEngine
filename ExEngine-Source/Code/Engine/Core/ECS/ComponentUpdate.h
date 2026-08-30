#pragma once
#include "string"
#include "nlohmann/json.hpp"
#include "../../JsonUtility/IJsonConvertable.h"

struct ComponentUpdate : public IJsonConvertable
{
public:
    unsigned int componentId;
    std::string fieldName;
    nlohmann::json newValue;

    ComponentUpdate() = default;
    ComponentUpdate(unsigned int componentId, const std::string& fieldName, nlohmann::json newValue)
        : componentId(componentId), fieldName(fieldName), newValue(std::move(newValue)) {};

    virtual nlohmann::json ToJson() override {
        return {
            {"componentId", componentId},
            {"fieldName", fieldName},
            {"newValue", newValue}
        };
    };

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("componentId")) componentId = json["componentId"].get<unsigned int>();
        if (json.contains("fieldName")) fieldName = json["fieldName"].get<std::string>();
        if (json.contains("newValue")) newValue = json["newValue"];
    };
};
