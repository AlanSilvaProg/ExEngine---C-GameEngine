#pragma once
#include <vector>
#include <deque>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "ECSSignature.h"
#include "Component/EComponentS.h"
#include "EntityCounter/EntityCSCounter.h"
#include "Pool/IPool.h"
#include "Pool/EComponentSPoolManager.h"

//Entity

class EntityCS{
private:
    unsigned int id;
    class ECSManager* ecsManager;

public:
    unsigned int GetId() { return id; };

    EntityCS() = default;
    EntityCS(const unsigned int id, ECSManager* ecsManager) : id(id), ecsManager(ecsManager){}
    
    void Kill();
    Signature GetComponentSignature();

    template<typename TComponent, typename ...TArgs>
    void AddComponent(TArgs&& ...args) const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent() const;
};


// System


class ECSystem {
protected:
    std::vector<int> systemSignatureIds;
    std::vector<EntityCS> systemEntities; 

    template<typename TComponent>
    void Require();

public:
    ECSystem(){
        SetupRequirements();
    };

    virtual ~ECSystem() = default;

    virtual void UpdateSystem(){};
    virtual void SetupRequirements(){};
    
    bool CheckEntitySignatureMatch(Signature entitySignature);
    void AddEntity(EntityCS entity);
    void ValidateEntity(EntityCS entity);
    void RemoveEntity(int id);
};


//Manager


class ECSManager{
private:
    std::vector<Signature> entitiesSignature; // each entity will have it own signature

    std::vector<EntityCS> entities;

    std::unordered_map<std::type_index, std::shared_ptr<ECSystem>> systems;

    std::vector<std::unique_ptr<IPool>> componentPools; // one pool by each component id 

    std::deque<int> entitiesToBeValidated; // validated to a system
    std::deque<int> entitiesToBeKilled; // removed from system and remove all components
    std::deque<int> freeEntities;

public:
    ECSManager();
    ~ECSManager() = default;


    void Update();

    EntityCS CreateEntity();
    void DestroyEntity(EntityCS entity);

    template<typename TComponent, typename ...TArgs>
    void AddComponent(EntityCS entity, TArgs&& ...args);
    template<typename TComponent>
    bool HasComponent(EntityCS entity) const;
    template<typename TComponent>
    void RemoveComponent(EntityCS entity);
    void RemoveAllComponents(EntityCS entity);
    Signature GetEntitySignature(const int id) const;

    template<typename TSystem, typename ...TArgs>
    std::shared_ptr<TSystem> CreateSystem(TArgs&& ...args);
    template<typename TSystem>
    std::shared_ptr<TSystem> GetSystem();
    template<typename TSystem>
    void UpdateSystem();

    void SetToValidation(int entityId);
};

template<typename TComponent, typename ...TArgs>
void ECSManager::AddComponent(EntityCS entity, TArgs&& ...args){
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];
    auto signatureSize = entitiesSignature.size();

    if(signatureSize <= componentId)
    {
        entitiesSignature.resize(signatureSize * 2); // making it power of two to improve performance ( less time resizing )
    }

    if(HasComponent<TComponent>(entity)) return;

    //Creating Component pool as needed

    if(componentPools.size() <= componentId)
        componentPools.resize(componentId * 2, nullptr);

    if(componentPools[componentId] == nullptr)
        componentPools[componentId] = std::make_unique<EComponentSPoolManager<TComponent>>();
        
    componentPools[componentId]->ComponentAddedToEntity(entityId, std::make_shared<TComponent>(forward(args)...));

    SetToValidation(entityId);
    entitySignature[componentId] = true;
};


template<typename TComponent>
bool ECSManager::HasComponent(EntityCS entity) const{

    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];

    return entitySignature[componentId];
};

template<typename TComponent>
void ECSManager::RemoveComponent(EntityCS entity){
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];

    if(!HasComponent<TComponent>(entity)) return;

    SetToValidation(entityId);
    entitySignature[componentId] = false;
};

template<typename TSystem, typename ...TArgs>
std::shared_ptr<TSystem> ECSManager::CreateSystem(TArgs&& ...args){
    if(systems.find(std::type_index(typeid(TSystem))) != systems.end()) return;

    auto newSystem = std::make_shared<TSystem>(forward(args)...);
    systems.insert(make_pair(std::type_index(typeid(TSystem)), static_cast<std::shared_ptr<ECSystem>>(newSystem)));
    return newSystem;
};

template<typename TSystem>
std::shared_ptr<TSystem> ECSManager::GetSystem(){
    return systems.find(std::type_index(typeid(TSystem)));
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
    ecsManager->AddComponent<TComponent>(*this, forward(args)...);
};

template<typename TComponent>
bool EntityCS::HasComponent() const{    
    ecsManager->HasComponent<TComponent>(*this);
};

template<typename TComponent>
void EntityCS::RemoveComponent() const{    
    ecsManager->RemoveComponent<TComponent>(*this);
};


//System


template<typename TComponent>
void ECSystem::Require(){
    auto componentId = EComponentS<TComponent>::GetId();
    systemSignatureIds.push_back(componentId);
};