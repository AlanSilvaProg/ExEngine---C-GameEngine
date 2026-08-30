#pragma once
#include "ECSSignature.h"
#include "Component/EComponentS.h"
#include "EntityCounter/EntityCSCounter.h"
#include "Pool/IPool.h"
#include "Pool/EComponentSPoolManager.h"
#include "EntityCounter/EntityCSCounter.h"
#include "ComponentUpdate.h"
#include "../../Logger/Logger.h"
#include <vector>
#include <deque>
#include <unordered_set>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

#ifndef TYPE_NAME
#define TYPE_NAME(T) #T
#endif

//Entity

class EntityCS{
private:
    unsigned int id;
    std::string guid;
    std::string name;

    class ECSManager* ecsManager;
public:
    unsigned int GetId() const { return id; };
    const std::string GetGuid() const { return guid; };

    void RegenerateGuid(std::string* newGuid = nullptr);

    EntityCS() = default;
    EntityCS(const unsigned int id, const std::string name, ECSManager* ecsManager) : id(id), name(name), ecsManager(ecsManager){
        RegenerateGuid();
    };

    void ChangeName(const std::string name);
    const std::string GetName() const;

    void UpdateComponentsByJson(const nlohmann::json& componentsContent);

    Signature& GetComponentSignature() const;

    template<typename TComponent, typename ...TArgs>
    std::shared_ptr<TComponent> AddComponent(TArgs&& ...args) const;

    template<typename TComponent>
    std::shared_ptr<TComponent> GetComponent() const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent() const;
    void RemoveComponent(const int componentId) const;
    void ApplyComponentUpdate(const ComponentUpdate& update) const;

    void KillImmediately();
    void Kill();
};


// System


enum SystemContext{
    EARLY_UPDATE,
    UPDATE,
    FIXED_UPDATE,
    LATE_UPDATE,
    PRE_RENDER,
    POST_RENDER
};

class ECSystem {
protected:
    std::vector<int> systemSignatureIds;
    std::vector<int> systemOptionalSignatureIds;
    std::vector<std::shared_ptr<EntityCS>> systemEntities; 

    template<typename TComponent>
    void Require(const bool optional);

    bool CheckForRegisteredId(const int componentId) const;

public:
    std::vector<std::shared_ptr<EntityCS>>* GetSystemEntities();
    std::vector<int>& GetRequirements(const bool getOptionals = false);
    bool CheckEntitySignatureMatch(const Signature& entitySignature) const;
    void AddEntity(const std::shared_ptr<EntityCS> entity);
    void ValidateEntity(std::shared_ptr<EntityCS> entity);
    void RemoveEntity(const int id);
    void ClearEntities();
    void RemoveRequirement(const int componentId);
    virtual void UpdateSystem(SystemContext systemContext){};

    virtual const char* SystemName() = 0; //adicionar nome para os sistemas
};


// Custom System


class CustomECSystem: public ECSystem{
private:
    std::string systemName;
    inline static u_int8_t id = 0;
    u_int8_t systemId;
public:
    CustomECSystem() { systemId = id ++;};
    CustomECSystem(std::string systemName) : systemName(systemName) { systemId = id ++; };
    ~CustomECSystem() = default;

    inline void SetSystemName(std::string name) { systemName = name; };
    virtual const char* SystemName() override { return systemName.c_str(); };

    template<typename TComponent>
    void IsRequired(const bool optional);
    template<typename TComponent>
    void IsNotRequired();

    inline u_int8_t GetId() { return systemId; };  

    template<typename TComponent>
    inline void AddRequire(const bool optional) { Require<TComponent>(optional); };
};


// System Context

struct SystemEntry {
    std::type_index type;
    std::shared_ptr<ECSystem> system;
};

class ECSystemContext{
private:
    SystemContext systemContext;
    std::vector<SystemEntry> systemEntries; //strong typed systems
    std::vector<std::shared_ptr<CustomECSystem>> customSystemEntries; //weak typed systems

    std::function<void()> removeEventHandlerCallback;

    void RefreshContext(SystemContext newContext);
public:
    bool enabled;

    inline ECSystemContext(SystemContext systemContext, bool enabled = true) : systemContext(systemContext), enabled(enabled) { RefreshContext(systemContext); };
    ~ECSystemContext() = default;

    void UpdateContext();
    void ValidateEntity(std::shared_ptr<EntityCS> entity);
    void Register(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem);
    void Unregister(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem);

    void RegisterCustom(std::shared_ptr<CustomECSystem> customECSystem);
    void UnregisterCustom(std::shared_ptr<CustomECSystem> customECSystem);

    inline const std::vector<SystemEntry>& GetContextSystems() const { return systemEntries; };
    inline const std::vector<std::shared_ptr<CustomECSystem>>& GetContextCustomSystems() const { return customSystemEntries; };
    inline const SystemContext GetSystemContext() { return systemContext; };
};


//Manager


class ECSManager{
private:
    std::vector<Signature> entitiesSignature; // each entity will have it own signature

    std::vector<std::shared_ptr<EntityCS>> entities;

    std::unordered_map<std::type_index, std::shared_ptr<ECSystem>> systems;
    std::vector<std::shared_ptr<CustomECSystem>> customECSystems;
    std::unordered_map<SystemContext, std::shared_ptr<ECSystemContext>> systemContext;

    std::vector<std::shared_ptr<IPool>> componentPools; // one pool by each component id 

    std::deque<int> entitiesToBeValidated; // validated to a system
    std::deque<int> entitiesToBeKilled; // removed from system and remove all components
    std::unordered_map<int,int> componentsToBeRemoved; // entityId, componentId
    std::deque<std::shared_ptr<ECSystem>> systemsToBeValidate; // revalidate all entities
    std::deque<int> freeEntities;

    std::unordered_set<int> aliveEntities;

    void LifeCycleCheck();
    void CreateSystemContexts();
public:
    ECSManager();
    ~ECSManager() = default;

    void Update();

    //entities
    std::shared_ptr<EntityCS> CreateEntity(const std::string entityName);
    std::shared_ptr<EntityCS> GetEntity(const int entityId); 
    void DuplicateEntity(const int entityId);
    void DestroyEntityImmediately(const int entityId);
    void DestroyEntity(const int entity);
    void DestroyEntity(const std::shared_ptr<EntityCS> entity);
    std::unordered_set<int>& GetAliveEntities();
    void DestroyAllEntities();
    void DestroyAllEntitiesImmediately();

    // components
    template<typename TComponent, typename ...TArgs>
    std::shared_ptr<TComponent> AddComponent(std::shared_ptr<EntityCS> entity, TArgs&& ...args);
    template<typename TComponent>
    std::shared_ptr<TComponent> GetComponent(std::shared_ptr<EntityCS> entity) const;
    template<typename TComponent>
    bool HasComponent(std::shared_ptr<EntityCS> entity) const;
    bool HasComponent(const int entityId, const int componentId);
    template<typename TComponent>
    void RemoveComponent(std::shared_ptr<EntityCS> entity);
    void RemoveComponent(const int entityId, const int componentId);
    void RemoveAllComponents(std::shared_ptr<EntityCS> entity);
    void UpdateEntityComponentsByJson(const int id, const nlohmann::json& componentsContent);
    bool ApplyComponentUpdate(const int entityId, const ComponentUpdate& update);
    Signature& GetEntitySignature(const int id);
    const std::vector<std::shared_ptr<IPool>>& GetEntityComponentPools() const;

    //systems
    const std::unordered_map<std::type_index, std::shared_ptr<ECSystem>>& GetAllSystems();
    const std::vector<std::shared_ptr<CustomECSystem>>& GetAllCustomSystems();
    const std::shared_ptr<ECSystemContext> GetECSystemContext(const SystemContext context) const;
    const void RevalidateSystem(std::shared_ptr<ECSystem> ecsystem);
    const void DestroyCustomECSystem(std::shared_ptr<CustomECSystem> customECSystem);
    void DestroySystem(const std::type_index typeIndex);

    template<typename TSystem, typename ...TArgs>
    std::shared_ptr<TSystem> CreateSystem(TArgs&& ...args);
    template<typename ...TArgs>
    std::shared_ptr<CustomECSystem> CreateCustomSystem(TArgs&& ...args);
    template<typename TSystem>
    std::shared_ptr<TSystem> GetSystem() const;

    void SetToValidation(const int entityId);
};

template<typename TComponent, typename ...TArgs>
std::shared_ptr<TComponent> ECSManager::AddComponent(std::shared_ptr<EntityCS> entity, TArgs&& ...args){
    auto componentId = EComponentS<TComponent>::GetId();
    auto componentCreationIndex = componentId + 1;
    auto entityId = entity->GetId();
    auto entitiesSignatureSize = entitiesSignature.size();

    if(entitiesSignatureSize <= entityId)
    {
        entitiesSignature.resize(EntityCSCounter::GetEntitiesCreated() * 2); // making it power of two to improve performance ( less time resizing )
    }

    auto entitySignature = &entitiesSignature[entityId];
    
    if(entitySignature->size() < componentCreationIndex)
    {
        entitySignature->resize(componentCreationIndex * 2, false);
    }

    if(HasComponent<TComponent>(entity)) return entity->GetComponent<TComponent>();
    
    //Creating Component pool as needed
    
    if(componentPools.size() <= componentCreationIndex)
        componentPools.resize(componentCreationIndex * 2, nullptr);

    if(componentPools[componentId] == nullptr)
        componentPools[componentId] = std::make_shared<EComponentSPoolManager>();

    auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPools[componentId]);

    auto component = std::make_shared<TComponent>(std::forward<TArgs>(args)...);

    castedPoolManager->ComponentAddedToEntity(entityId, component);

    SetToValidation(entityId);
    entitiesSignature[entityId][componentId] = true;
    Logger::Log("Component Id: " + std::to_string(componentId) + "Added To Entity ID: " + std::to_string(entityId));
    return component;
};

template<typename TComponent>
std::shared_ptr<TComponent> ECSManager::GetComponent(std::shared_ptr<EntityCS> entity) const{
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity->GetId();
    auto entitySignature = entitiesSignature[entityId];

    if(entitySignature.size() > componentId && entitySignature[componentId])
    {
        auto component = componentPools[componentId];
        if(component != nullptr)
        {
            auto convertedPool = std::dynamic_pointer_cast<EComponentSPoolManager>(component);
            auto component = convertedPool->GetComponent(entityId);
            
            if(component != nullptr) return std::dynamic_pointer_cast<TComponent>(component);
        }
    }

    return NULL;
};

template<typename TComponent>
bool ECSManager::HasComponent(std::shared_ptr<EntityCS> entity) const{
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity->GetId();
    auto entitySignature = entitiesSignature[entityId];

    if(entitySignature.size() <= componentId)
    {
        return false;
    }

    return entitySignature[componentId];
};

template<typename TComponent>
void ECSManager::RemoveComponent(std::shared_ptr<EntityCS> entity){
    auto entityId = entity->GetId();
    auto entitySignature = entitiesSignature[entityId];

    if(!HasComponent<TComponent>(entity)) return;

    auto componentId = EComponentS<TComponent>::GetId();

    SetToValidation(entityId);
    entitySignature[componentId] = false;
};

template<typename TSystem, typename ...TArgs>
std::shared_ptr<TSystem> ECSManager::CreateSystem(TArgs&& ...args){
    if(systems.find(std::type_index(typeid(TSystem))) != systems.end()) return nullptr;

    auto newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    auto castedNewSystem = static_cast<std::shared_ptr<ECSystem>>(newSystem);
    systems.insert(make_pair(std::type_index(typeid(TSystem)), castedNewSystem));

    RevalidateSystem(newSystem);
    Logger::Log("ECS System Created: " + std::string(castedNewSystem->SystemName()));

    return newSystem;
};

template<typename ...TArgs>
std::shared_ptr<CustomECSystem> ECSManager::CreateCustomSystem(TArgs&& ...args){
    auto newSystem = std::make_shared<CustomECSystem>(std::forward<TArgs>(args)...);
    customECSystems.push_back(newSystem);

    RevalidateSystem(newSystem);
    Logger::Log("ECS Custom System Created: " + std::string(newSystem->SystemName()));

    return newSystem;
};

template<typename TSystem>
std::shared_ptr<TSystem> ECSManager::GetSystem()const {
    auto it = systems.find(std::type_index(typeid(TSystem)));
    if (it != systems.end()) {
        return std::dynamic_pointer_cast<TSystem>(it->second);
    }
    return nullptr; // ou lançar exceção, conforme seu caso
};


//Entity


template<typename TComponent, typename ...TArgs>
std::shared_ptr<TComponent> EntityCS::AddComponent(TArgs&& ...args) const{    
    return ecsManager->AddComponent<TComponent>(ecsManager->GetEntity(id), std::forward<TArgs>(args)...);
};

template<typename TComponent>
std::shared_ptr<TComponent> EntityCS::GetComponent() const{
    return ecsManager->GetComponent<TComponent>(ecsManager->GetEntity(id));
};

template<typename TComponent>
bool EntityCS::HasComponent() const{    
    return ecsManager->HasComponent<TComponent>(*this);
};

template<typename TComponent>
void EntityCS::RemoveComponent() const{    
    ecsManager->RemoveComponent<TComponent>(*this);
};


//


template<typename TComponent>
void CustomECSystem::IsRequired(const bool optional){ 
    Require<TComponent>(optional);
};

template<typename TComponent>
void CustomECSystem::IsNotRequired(){
    auto componentId = EComponentS<TComponent>::GetId();

    if(!CheckForRegisteredId(componentId)) return;

    bool erased = false;
    systemOptionalSignatureIds.erase(
        std::remove_if(
            systemOptionalSignatureIds.begin(),
            systemOptionalSignatureIds.end(),
            [componentId, &erased](int value) 
            {
                auto condition = value == componentId;
                if(condition) erased = true;
                return condition; 
            }
        ),
        systemOptionalSignatureIds.end()
    );

    if(erased) return;

    systemSignatureIds.erase(
        std::remove_if(
            systemSignatureIds.begin(),
            systemSignatureIds.end(),
            [componentId](int value) 
            {
                return value == componentId; 
            }
        ),
        systemSignatureIds.end()
    );
};


//System


template<typename TComponent>
void ECSystem::Require(const bool optional){
    auto componentId = EComponentS<TComponent>::GetId();

    if(CheckForRegisteredId(componentId)) return;

    if(optional)
    {
        systemOptionalSignatureIds.push_back(componentId);
        return;
    }

    systemSignatureIds.push_back(componentId);
};