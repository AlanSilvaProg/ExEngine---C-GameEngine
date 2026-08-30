#pragma once
#include <vector>
#include "../../JsonUtility/IJsonConvertable.h"
#include "../ECS/ComponentUpdate.h"

class EntityContent : public IJsonConvertable{
public:
    std::vector<ComponentUpdate> componentUpdates;

    inline virtual nlohmann::json ToJson() override{
        nlohmann::json componentUpdatesJson = nlohmann::json::array();
        for (auto& componentUpdate : componentUpdates) {
            componentUpdatesJson.push_back(componentUpdate.ToJson());
        }

        return {
            {"componentUpdates", componentUpdatesJson}
        };
    };

    inline virtual void FromJson(const nlohmann::json& json) override{
        componentUpdates.clear();

        if (json.contains("componentUpdates")) {
            componentUpdates.reserve(json["componentUpdates"].size());

            for (const auto& componentUpdateJson : json["componentUpdates"]) {
                ComponentUpdate componentUpdate;
                componentUpdate.FromJson(componentUpdateJson);
                componentUpdates.push_back(std::move(componentUpdate));
            }
        }
    };
};