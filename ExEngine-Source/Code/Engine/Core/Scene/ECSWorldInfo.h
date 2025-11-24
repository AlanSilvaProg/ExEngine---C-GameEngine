#pragma once
#include "../../JsonUtility/IJsonConvertable.h"
#include "DataContainer/EntityContainer.h"
#include <string>
#include <vector>
#include <cstdint>

struct ECSWorldInfo: public IJsonConvertable{
public:
    std::string name;
    uint32_t worldHash;
    std::vector<EntityContainer> entityContainer;

    ECSWorldInfo() = default;
    ~ECSWorldInfo() = default;

    virtual nlohmann::json ToJson() override {
        nlohmann::json arr = nlohmann::json::array();
        for (auto& e : entityContainer)
            arr.push_back(e.ToJson());
            
        return {
            {"name", name},
            {"entityContainer", arr}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("name")) name = json["name"];
        
        if (json.contains("entityContainer")) 
        {
            entityContainer.clear();
            for (auto& item : json["entityContainer"]) {
                EntityContainer e;
                e.FromJson(item);
                entityContainer.push_back(e);
            }
        }
    }
};