#include "ECSManager.h"
#include "../../Logger/Logger.h"
#include <new>

ECSManager::ECSManager(){
    
};

void ECSManager::LifeCycleCheck(){
    if(componentsToBeRemoved.size() > 0)
    {
        for(auto pair : componentsToBeRemoved)
        {
            const int entityId = pair.first;
            const int componentId = pair.second;

            auto& entitySignature = GetEntitySignature(entityId);
            auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPools[componentId]);

            entitySignature[componentId] = false;
            castedPoolManager->ComponentRemovedFromEntity(pair.first); // ToDo undo command

            SetToValidation(entityId);
            Logger::Log("Component : " + std::to_string(componentId) + " removed from entity: " + std::to_string(entityId));
        }
        componentsToBeRemoved.clear();
    }

    if(entitiesToBeKilled.size() > 0)
    {
        for(auto entityId : entitiesToBeKilled){
            RemoveAllComponents(entities[entityId]);
            aliveEntities.erase(entityId);
            freeEntities.push_back(entityId);
            
            Logger::Log("Entity with ID: " + std::to_string(entityId) + " has been killed.");
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

void ECSManager::Update(){
    LifeCycleCheck();
};

EntityCS& ECSManager::CreateEntity(const std::string entityName, const bool internal){
    if(freeEntities.empty()){
        EntityCS entity(EntityCSCounter::GetEntitiesCreated(), entityName, this, internal);
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
    auto& entitySignature = entitiesSignature[entityId];

    for(int i = 0; i < entitySignature.size(); i ++)
    {
        if(HasComponent(entityId, i))
        {
            entitySignature[i] = false; 
            auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPools[i]);
            castedPoolManager->ComponentRemovedFromEntity(entityId); // ToDo undo command
        }
    }

    SetToValidation(entityId);
};

void ECSManager::RemoveComponent(const int entityId, const int componentId){
    componentsToBeRemoved.emplace(entityId, componentId);
};

bool ECSManager::HasComponent(const int entityId, const int componentId){
    return GetEntitySignature(entityId)[componentId];
};

Signature& ECSManager::GetEntitySignature(const int id){
    return entitiesSignature[id];
};

const std::vector<std::shared_ptr<IPool>>& ECSManager::GetEntityComponentPools() const{
    return componentPools;
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

const std::unordered_map<std::type_index, std::shared_ptr<ECSystem>>& ECSManager::GetAllSystems(){
    return systems;
};

void ECSManager::DestroyAllEntities(){
    for(auto entityId : aliveEntities)
    {
        DestroyEntity(*GetEntity(entityId));
    }
};

void ECSManager::DestroyAllEntitiesImmediately(){
    DestroyAllEntities();
    LifeCycleCheck();
};

//Entity

void EntityCS::ChangeName(const std::string name){
    this->name = name;
};

Signature& EntityCS::GetComponentSignature() const{
    return ecsManager->GetEntitySignature(GetId());
};

void EntityCS::RemoveComponent(const int componentId) const{
    ecsManager->RemoveComponent(GetId(), componentId);
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

bool ECSystem::CheckEntitySignatureMatch(const Signature& entitySignature) const{
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