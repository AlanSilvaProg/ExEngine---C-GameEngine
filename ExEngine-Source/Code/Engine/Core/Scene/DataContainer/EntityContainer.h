#pragma once
#include "../../ECS/ECSManager.h"
#include "../../../JsonUtility/IJsonConvertable.h"
#include "../Engine/Core/EngineGetters.h"

struct EntityContainer : public IJsonConvertable{
public:
    std::string name;
    std::string guid;
    // Empty when the entity has no parent. Stored as a GUID (not the parent's numeric id, which
    // isn't stable across a save/load - ids get reassigned/recycled by ECSManager) and resolved
    // back to a runtime id by ECSWorld once every entity in the world has been created.
    std::string parentGuid;
    nlohmann::json components;
    int currentId;

    EntityContainer() = default;

    inline EntityContainer(std::shared_ptr<EntityCS> entityCS) {
        name = entityCS->GetName();
        currentId = entityCS->GetId();
        guid = entityCS->GetGuid();

        auto ecsManager = EngineGetters::GetEnginePtr()->GetECSManagerPtr();

        auto parent = entityCS->GetParent();
        parentGuid = parent != nullptr ? parent->GetGuid() : "";

        const auto componentsPool = ecsManager->GetEntityComponentPools();

        components = nlohmann::json::array();

        for(const auto pool : componentsPool)
        {
            auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pool);

            if(castedPoolManager == nullptr) continue;

            //getting entity component 
            const auto component = castedPoolManager->GetComponent(currentId);

            if(component == nullptr) continue;

            if(!ecsManager->HasComponent(currentId, component->GetComponentId())) continue;

            nlohmann::json entry;
            entry["component"] = component->GetComponentName();
            entry["id"] = component->GetComponentId();
            entry["data"] = component->ToJson();
            
            components.push_back(entry);
        }
    };

    ~EntityContainer() = default;

    virtual nlohmann::json ToJson() override {
        return {
            {"name", name},
            {"guid", guid},
            {"parentGuid", parentGuid},
            {"components", components}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("name")) name = json["name"];
        if (json.contains("guid")) guid = json["guid"];
        if (json.contains("parentGuid")) parentGuid = json["parentGuid"];
        if (json.contains("components")) components = json["components"];
    }
};