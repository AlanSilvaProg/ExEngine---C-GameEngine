#pragma once
#include "ECSSignature.h"
#include "Component/EComponentS.h"
#include "EntityCounter/EntityCSCounter.h"
#include "Pool/IPool.h"
#include "Pool/EComponentSPoolManager.h"
#include "EntityCounter/EntityCSCounter.h"
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
    bool internal;
    unsigned int id;
    std::string name;

    class ECSManager* ecsManager;
public:
    unsigned int GetId() const { return id; };

    EntityCS() = default;
    EntityCS(const unsigned int id, const std::string name, ECSManager* ecsManager, const bool internal = false) : id(id), name(name), ecsManager(ecsManager), internal(internal){}

    void ChangeName(const std::string name);
    const std::string GetName() const;

    void Kill();
    Signature& GetComponentSignature() const;

    template<typename TComponent, typename ...TArgs>
    void AddComponent(TArgs&& ...args) const;

    template<typename TComponent>
    std::shared_ptr<TComponent> GetComponent() const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent() const;
    void RemoveComponent(const int componentId) const;

    inline bool IsInternal() { return internal; }

};


// System


class ECSystem {
protected:
    std::vector<int> systemSignatureIds;
    std::vector<int> systemOptionalSignatureIds;
    std::vector<EntityCS> systemEntities; 

    template<typename TComponent>
    void Require(const bool optional);

    bool CheckForRegisteredId(const int componentId) const;

public:
    std::vector<EntityCS>* GetSystemEntities();
    bool CheckEntitySignatureMatch(const Signature& entitySignature) const;
    void AddEntity(const EntityCS entity);
    void ValidateEntity(EntityCS entity);
    void RemoveEntity(const int id);
    virtual void UpdateSystem(){};

    virtual const char* SystemName() = 0; //adicionar nome para os sistemas
};


// Custom System


class CustomECSystem: public ECSystem{
private:
    std::string systemName;
public:
    CustomECSystem() = default;
    ~CustomECSystem() = default;

    inline void SetSystemName(std::string name) { systemName = name; };
    inline std::string GetSystemName() const { return systemName; };

    template<typename TComponent>
    void IsRequired(const bool optional);
    template<typename TComponent>
    void IsNotRequired();
};


// System Context


enum SystemContext{
    EARLY_UPDATE,
    UPDATE,
    FIXED_UPDATE,
    LATE_UPDATE,
    PRE_RENDER,
    POST_RENDER
};

struct SystemEntry {
    std::type_index type;
    std::shared_ptr<ECSystem> system;
};

class ECSystemContext{
private:
    SystemContext systemContext;
    std::vector<SystemEntry> systemEntries;

    bool internal;
    std::function<void()> removeEventHandlerCallback;

    void RefreshContext(SystemContext newContext);
public:
    inline ECSystemContext(SystemContext systemContext) : systemContext(systemContext) { RefreshContext(systemContext); };
    ~ECSystemContext() = default;

    void UpdateContext();
    void ValidateEntity(EntityCS entity);
    void Register(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem);
    void Unregister(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem);

    const void SetSystemContext(SystemContext context);
    inline const std::vector<SystemEntry>& GetContextSystems() const { return systemEntries; };
    inline const SystemContext GetSystemContext() { return systemContext; };
    inline const bool IsInternal() const { return internal; };
};


//Manager


class ECSManager{
private:
    std::vector<Signature> entitiesSignature; // each entity will have it own signature

    std::vector<EntityCS> entities;

    std::unordered_map<std::type_index, std::shared_ptr<ECSystem>> systems;

    std::vector<std::shared_ptr<IPool>> componentPools; // one pool by each component id 

    std::deque<int> entitiesToBeValidated; // validated to a system
    std::deque<int> entitiesToBeKilled; // removed from system and remove all components
    std::unordered_map<int,int> componentsToBeRemoved; // entityId, componentId
    std::deque<int> freeEntities;

    std::unordered_set<int> aliveEntities;

    void LifeCycleCheck();
public:
    ECSManager();
    ~ECSManager() = default;

    void Update();

    EntityCS& CreateEntity(const std::string entityName, const bool internal = false);
    EntityCS* GetEntity(const int entityId); 
    void DestroyEntity(EntityCS entity);
    std::unordered_set<int>& GetAliveEntities();
    void DestroyAllEntities();
    void DestroyAllEntitiesImmediately();

    template<typename TComponent, typename ...TArgs>
    void AddComponent(EntityCS entity, TArgs&& ...args);
    template<typename TComponent>
    std::shared_ptr<TComponent> GetComponent(EntityCS entity) const;
    template<typename TComponent>
    bool HasComponent(EntityCS entity) const;
    bool HasComponent(const int entityId, const int componentId);
    template<typename TComponent>
    void RemoveComponent(EntityCS entity);
    void RemoveComponent(const int entityId, const int componentId);
    void RemoveAllComponents(EntityCS entity);
    Signature& GetEntitySignature(const int id);
    const std::vector<std::shared_ptr<IPool>>& GetEntityComponentPools() const;

    const std::unordered_map<std::type_index, std::shared_ptr<ECSystem>>& GetAllSystems();
    template<typename TSystem, typename ...TArgs>
    std::shared_ptr<TSystem> CreateSystem(TArgs&& ...args);
    template<typename TSystem>
    std::shared_ptr<TSystem> GetSystem() const;
    template<typename TSystem>
    void UpdateSystem();

    void SetToValidation(const int entityId);
};

template<typename TComponent, typename ...TArgs>
void ECSManager::AddComponent(EntityCS entity, TArgs&& ...args){
    auto componentId = EComponentS<TComponent>::GetId();
    auto componentCreationIndex = componentId + 1;
    auto entityId = entity.GetId();
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

    if(HasComponent<TComponent>(entity)) return;
    
    //Creating Component pool as needed
    
    if(componentPools.size() <= componentCreationIndex)
        componentPools.resize(componentCreationIndex * 2, nullptr);

    if(componentPools[componentId] == nullptr)
        componentPools[componentId] = std::make_shared<EComponentSPoolManager>();

    auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPools[componentId]);

    castedPoolManager->ComponentAddedToEntity(entityId, std::make_shared<TComponent>(std::forward<TArgs>(args)...));

    SetToValidation(entityId);
    entitiesSignature[entityId][componentId] = true;
};

template<typename TComponent>
std::shared_ptr<TComponent> ECSManager::GetComponent(EntityCS entity) const{
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
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
bool ECSManager::HasComponent(EntityCS entity) const{
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];

    if(entitySignature.size() <= componentId)
    {
        return false;
    }

    return entitySignature[componentId];
};

template<typename TComponent>
void ECSManager::RemoveComponent(EntityCS entity){
    auto entityId = entity.GetId();
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

    Logger::Log("ECS System Created: " + std::string(castedNewSystem->SystemName()));

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

template<typename TSystem>
void ECSManager::UpdateSystem(){
    auto system = GetSystem<TSystem>();
    if(system != systems.end())
    {
        static_cast<std::shared_ptr<ECSystem>>(system)->UpdateSystem();
    }
};


//Entity


template<typename TComponent, typename ...TArgs>
void EntityCS::AddComponent(TArgs&& ...args) const{    
    ecsManager->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
};

template<typename TComponent>
std::shared_ptr<TComponent> EntityCS::GetComponent() const{
    return ecsManager->GetComponent<TComponent>(*this);
};

template<typename TComponent>
bool EntityCS::HasComponent() const{    
    ecsManager->HasComponent<TComponent>(*this);
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