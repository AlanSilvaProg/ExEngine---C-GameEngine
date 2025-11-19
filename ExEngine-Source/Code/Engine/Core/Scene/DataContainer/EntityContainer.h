#pragma once
#include "../../ECS/ECSManager.h"
#include "../../../JsonUtility/IJsonConvertable.h"

struct EntityContainer : public IJsonConvertable{
public:
    bool internal;
    //ToDo recreate components and values
    std::string name;

    EntityContainer() = default;

    inline EntityContainer(EntityCS entityCS) {
        internal = entityCS.IsInternal();
        name = entityCS.GetName();
    };

    ~EntityContainer() = default;

    virtual nlohmann::json ToJson() override {
        return {
            {"internal", internal},
            {"name", name},
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("internal")) internal = json["internal"];
        if (json.contains("name")) name = json["name"];
    }
};