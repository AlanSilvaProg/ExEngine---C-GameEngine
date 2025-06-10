#include "ECSManager.h"
#include "../../Logger/Logger.h"
#include <new>

ECSManager::ECSManager(){
    
};

void ECSManager::Update(){
    if(entitiesToBeKilled.size() > 0)
    {
        for(auto entityId : entitiesToBeKilled){
            RemoveAllComponents(entities[entityId]);
            aliveEntities.erase(entityId);
        }

        entitiesToBeKilled.clear();
    }

    if(entitiesToBeValidated.size() > 0)
    {
        for(auto system : systems)
        {
            for(auto entityId : entitiesToBeValidated)
            {
                system.second->ValidateEntity(entities[entityId]);
            }
        }

        entitiesToBeValidated.clear();
    }
};

EntityCS& ECSManager::CreateEntity(const std::string entityName){
    if(freeEntities.empty()){
        EntityCS entity(EntityCSCounter::GetEntitiesCreated(), entityName, this);
        auto entityId = entity.GetId();

        auto entitiesCreated = EntityCSCounter::IncreaseEntitiesCreated();

        if(entities.size() <= entitiesCreated){
            aliveEntities.reserve(entitiesCreated * 2);
            entities.resize(entitiesCreated * 2);
            entitiesSignature.resize(entities.size());
        }

        entities[entityId] = entity;
        Logger::Log("Entity created with ID: " + std::to_string(entityId));

        aliveEntities.insert(entityId);
        SetToValidation(entityId);

        return entities[entityId];
    }

    auto entityId = freeEntities.front();

    entities[entityId].ChangeName(entityName);
    aliveEntities.insert(entityId);
    SetToValidation(entityId);
    freeEntities.pop_front();

    Logger::Log("Entity created with a recycled ID: " + std::to_string(entityId));

    return entities[entityId];
};

EntityCS* ECSManager::GetEntity(const int entityId){
    if(entityId > EntityCSCounter::GetEntitiesCreated())
    {
        Logger::LogError("Entity wasn't created yet, ID: " + std::to_string(entityId));
        return nullptr;
    }

    return &entities[entityId];
};

void ECSManager::DestroyEntity(EntityCS entity){
    entitiesToBeKilled.emplace_back(entity.GetId());
};

void ECSManager::RemoveAllComponents(EntityCS entity){
    auto entityId = entity.GetId();
    auto entitySignature = entitiesSignature[entityId];

    for(int i = 0; i < entitySignature.size(); i ++){
        entitySignature[i] = false;
    }

    SetToValidation(entityId);
};


Signature ECSManager::GetEntitySignature(const int id) const{
    return entitiesSignature[id];
};

std::unordered_set<int>& ECSManager::GetAliveEntities(){
    return aliveEntities;
};

void ECSManager::SetToValidation(const int entityId){
    auto needValidation = true;
    for(auto validationId : entitiesToBeValidated){
        if(validationId == entityId)
        {
            needValidation = false;
            break;
        }
    }

    if(needValidation)
        entitiesToBeValidated.push_back(entityId);
};


//Entity

void EntityCS::ChangeName(const std::string name){
    this->name = name;
};

Signature EntityCS::GetComponentSignature() const{
    return ecsManager->GetEntitySignature(GetId());
};

void EntityCS::Kill(){
    ecsManager->DestroyEntity(*this);
};

const std::string EntityCS::GetName() const{
    return name;
};


//System

std::vector<EntityCS>* ECSystem::GetSystemEntities(){
    return &systemEntities;
};

bool ECSystem::CheckEntitySignatureMatch(const Signature entitySignature) const{
    for(auto signatureId : systemSignatureIds)
    {
        if(entitySignature.size() <= signatureId || !entitySignature[signatureId])
            return false;
    }
    return true;
};

void ECSystem::AddEntity(const EntityCS entity){
    systemEntities.push_back(entity);
};

void ECSystem::ValidateEntity(EntityCS entity)
{
    auto entityId = entity.GetId();
    for(int i = 0; i < systemEntities.size(); i++)
    {
        if(systemEntities[i].GetId() == entityId)
        {
            if(!CheckEntitySignatureMatch(entity.GetComponentSignature()))
            {
                RemoveEntity(entityId);
            }
            return;
        }
    }

    if(CheckEntitySignatureMatch(entity.GetComponentSignature()))
    {
        AddEntity(entity);
    }
};

void ECSystem::RemoveEntity(const int id){
    systemEntities.erase(std::remove_if(systemEntities.begin(), systemEntities.end(),
                        [id](EntityCS entity) { return entity.GetId() == id; }),
                        systemEntities.end());
};

bool ECSystem::CheckForRegisteredId(const int componentId, const bool optional) const{
    for(auto id : systemOptionalSignatureIds)
    {
        if(id == componentId)
        {
            Logger::LogWarning("Same component has been added multiple times for system requirements at " + std::to_string(*typeid(*this).name()));
            return true;
        }
    }
    return false;
};