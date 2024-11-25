#pragma once
#include <vector>
#include <deque>
#include "ECSSignature.h"

unsigned int previousId = 0;

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

    template<typename TComponent>
    void AddComponent() const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent() const;
};


//Component


struct IEComponentS {
protected:
    static unsigned int previousId;
};

template<typename T>
class EComponentS : public IEComponentS{
public:
    static unsigned int GetId(){
        static auto id = previousId + 1;
        return id;
    };
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
    unsigned int enitiesCreated; 

    std::vector<ECSystem> systems;

    std::deque<int> entitiesToBeValidated; // validated to a system
    std::deque<int> entitiesToBeKilled; // removed from system and remove all components
    std::deque<int> freeEntities;

public:
    void Update();

    EntityCS CreateEntity();
    void DestroyEntity(EntityCS entity);

    template<typename TComponent>
    void AddComponent(EntityCS entity);
    template<typename TComponent>
    bool HasComponent(EntityCS entity) const;
    template<typename TComponent>
    void RemoveComponent(EntityCS entity);
    void RemoveAllComponents(EntityCS entity);
    Signature GetEntitySignature(const int id) const;

    template<typename TSystem>
    void CreateSystem();

    void SetToValidation(int entityId);
};

template<typename TComponent>
void ECSManager::AddComponent(EntityCS entity){
    auto componentId = EComponentS<TComponent>::GetId();
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];
    auto signatureSize = entitiesSignature.size();

    if(signatureSize <= componentId)
    {
        entitiesSignature.resize(signatureSize * 2); // making it power of two to improve performance ( less time resizing )
    }

    if(HasComponent<TComponent>(entity)) return;

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


template<typename TSystem>
void CreateSystem(){
    //ToDo Create System
};


//Entity


template<typename TComponent>
void EntityCS::AddComponent() const{    
    ecsManager->AddComponent<TComponent>(*this);
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