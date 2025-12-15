#include "ECSManager.h"
#include "../../Logger/Logger.h"
#include "../Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "../Serializer/Demangle.h"
#include "../UID/UID.h"
#include <new>
#include <memory>
#include <algorithm>

ECSManager::ECSManager(){
    CreateSystemContexts();
};
    
void ECSManager::CreateSystemContexts(){
    systemContext.emplace(SystemContext::EARLY_UPDATE, std::make_shared<ECSystemContext>(SystemContext::EARLY_UPDATE));
    systemContext.emplace(SystemContext::UPDATE, std::make_shared<ECSystemContext>(SystemContext::UPDATE));
    systemContext.emplace(SystemContext::FIXED_UPDATE, std::make_shared<ECSystemContext>(SystemContext::FIXED_UPDATE));
    systemContext.emplace(SystemContext::LATE_UPDATE, std::make_shared<ECSystemContext>(SystemContext::LATE_UPDATE));
    systemContext.emplace(SystemContext::PRE_RENDER, std::make_shared<ECSystemContext>(SystemContext::PRE_RENDER));
    systemContext.emplace(SystemContext::POST_RENDER, std::make_shared<ECSystemContext>(SystemContext::POST_RENDER));
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
            DestroyEntityImmediately(entityId);
        }

        entitiesToBeKilled.clear();
    }

    if(!entitiesToBeValidated.empty())
    {
        for(const auto entityId : entitiesToBeValidated)
        {
            const auto& entity = entities[entityId];
            for(const auto& [typeIndex, system] : systems)
            {
                system->ValidateEntity(entity);
            }

            for(const auto& system : customECSystems)
            {   
                system->ValidateEntity(entity);
            }
        }

        entitiesToBeValidated.clear();
    }

    if(!systemsToBeValidate.empty())
    {
        for(auto ecsystem : systemsToBeValidate)
        {
            ecsystem->ClearEntities();

            for(auto entityId : aliveEntities)
            {
                ecsystem->ValidateEntity(GetEntity(entityId));
            }
        }

        systemsToBeValidate.clear();
    }
};

void ECSManager::Update(){
    LifeCycleCheck();
};

std::shared_ptr<EntityCS> ECSManager::CreateEntity(const std::string entityName, const bool internal){
    if(freeEntities.empty()){
        auto entity = std::make_shared<EntityCS>(EntityCSCounter::GetEntitiesCreated(), entityName, this, internal);
        auto entityId = entity->GetId();

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

    entities[entityId]->ChangeName(entityName);
    aliveEntities.insert(entityId);
    SetToValidation(entityId);
    freeEntities.pop_front();

    Logger::Log("Entity created with a recycled ID: " + std::to_string(entityId));

    return entities[entityId];
};

std::shared_ptr<EntityCS> ECSManager::GetEntity(const int entityId){
    if(entityId > EntityCSCounter::GetEntitiesCreated())
    {
        Logger::LogError("Entity wasn't created yet, ID: " + std::to_string(entityId));
        return nullptr;
    }

    return entities[entityId];
};

void ECSManager::DestroyEntityImmediately(int entityId){
    RemoveAllComponents(entities[entityId]);
    aliveEntities.erase(entityId);
    freeEntities.push_back(entityId);
    
    Logger::Log("Entity with ID: " + std::to_string(entityId) + " has been killed.");
};

void ECSManager::DestroyEntity(std::shared_ptr<EntityCS> entity){
    entitiesToBeKilled.emplace_back(entity->GetId());
};

void ECSManager::RemoveAllComponents(std::shared_ptr<EntityCS> entity){
    auto entityId = entity->GetId();
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

const std::vector<std::shared_ptr<CustomECSystem>>& ECSManager::GetAllCustomSystems(){
    return customECSystems;
};

const std::shared_ptr<ECSystemContext> ECSManager::GetECSystemContext(const SystemContext context) const{
    for(auto ctxt : systemContext)
    {
        if(ctxt.first == context) return ctxt.second;
    }

    return nullptr;
};

const void ECSManager::RevalidateSystem(std::shared_ptr<ECSystem> ecsystem){
    systemsToBeValidate.emplace_back(ecsystem);
};

const void ECSManager::DestroyCustomECSystem(std::shared_ptr<CustomECSystem> customECSystem){
    std::erase_if(customECSystems, [&](const std::shared_ptr<CustomECSystem> ecsystem){ return ecsystem->GetId() == customECSystem->GetId(); });
};

void ECSManager::DestroyAllEntities(){
    for(auto entityId : aliveEntities)
    {
        DestroyEntity(GetEntity(entityId));
    }
};

void ECSManager::DestroyAllEntitiesImmediately(){
    DestroyAllEntities();
    LifeCycleCheck();
};

//Entity

void EntityCS::RegenerateGuid(std::string* newGuid){
    if(newGuid != nullptr)
    {
        guid = *newGuid;
        return;
    }

    guid = UID::GenerateGUID();
};

void EntityCS::ChangeName(const std::string name){
    this->name = name;
};

Signature& EntityCS::GetComponentSignature() const{
    return ecsManager->GetEntitySignature(GetId());
};

void EntityCS::RemoveComponent(const int componentId) const{
    ecsManager->RemoveComponent(GetId(), componentId);
};

void EntityCS::KillImmediately(){
    ecsManager->DestroyEntityImmediately(id);
};

void EntityCS::Kill(){
    ecsManager->DestroyEntity(ecsManager->GetEntity(id));
};

const std::string EntityCS::GetName() const{
    return name;
};

//System

std::vector<std::shared_ptr<EntityCS>>* ECSystem::GetSystemEntities(){
    return &systemEntities;
};

std::vector<int>& ECSystem::GetRequirements(const bool getOptionals){
    if(getOptionals)
    {
        return systemOptionalSignatureIds;
    }
    
    return systemSignatureIds;
};

bool ECSystem::CheckEntitySignatureMatch(const Signature& entitySignature) const{
    for(auto signatureId : systemSignatureIds)
    {
        if(entitySignature.size() <= signatureId || !entitySignature[signatureId])
            return false;
    }
    return true;
};

void ECSystem::AddEntity(const std::shared_ptr<EntityCS> entity){
    systemEntities.push_back(entity);
};

void ECSystem::ValidateEntity(std::shared_ptr<EntityCS> entity)
{
    auto entityId = entity->GetId();
    for(int i = 0; i < systemEntities.size(); i++)
    {
        if(systemEntities[i]->GetId() == entityId)
        {
            if(!CheckEntitySignatureMatch(entity->GetComponentSignature()))
            {
                RemoveEntity(entityId);
            }
            return;
        }
    }

    if(CheckEntitySignatureMatch(entity->GetComponentSignature()))
    {
        AddEntity(entity);
    }
};

void ECSystem::RemoveEntity(const int id){
    systemEntities.erase(std::remove_if(systemEntities.begin(), systemEntities.end(),
                        [id](std::shared_ptr<EntityCS> entity) { return entity->GetId() == id; }),
                        systemEntities.end());
};

void ECSystem::ClearEntities(){
    systemEntities.clear();
};

bool ECSystem::CheckForRegisteredId(const int componentId) const{
    for(auto id : systemOptionalSignatureIds)
    {
        if(id == componentId)
        {
            Logger::LogWarning("Same component has been added multiple times for system requirements at " + Demangle(typeid(*this).name()));
            return true;
        }
    }

    for(auto id : systemSignatureIds)
    {
        if(id == componentId)
        {
            Logger::LogWarning("Same component has been added multiple times for system requirements at " + Demangle(typeid(*this).name()));
            return true;
        }
    }
    return false;
};

// System Context

void ECSystemContext::UpdateContext(){
    for(auto& systemEntry : systemEntries)
    {
        systemEntry.system->UpdateSystem();
    }

    for(auto& systemEntry : customSystemEntries)
    {
        systemEntry->UpdateSystem();
    }
};

void ECSystemContext::ValidateEntity(std::shared_ptr<EntityCS> entity){
    for(auto& systemEntry : systemEntries)
    {
        systemEntry.system->ValidateEntity(entity);
    }
};

void ECSystemContext::Register(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem){
    systemEntries.push_back({typeIndex, ecsSystem});
};

void ECSystemContext::Unregister(const std::type_index typeIndex, std::shared_ptr<ECSystem> ecsSystem){
    for (size_t i = 0; i < systemEntries.size(); i++) {
        auto& systemEntry = systemEntries[i];
        if (systemEntry.type == typeIndex && systemEntry.system == ecsSystem) {
            systemEntries.erase(systemEntries.begin() + i);
            return;
        }
    }
};

void ECSystemContext::RegisterCustom(std::shared_ptr<CustomECSystem> customECSystem){
    customSystemEntries.push_back(customECSystem);
};

void ECSystemContext::UnregisterCustom(std::shared_ptr<CustomECSystem> customECSystem){
    std::erase_if(customSystemEntries, [&](const auto element){ return element->GetId() == customECSystem->GetId(); });
};

void ECSystemContext::RefreshContext(SystemContext newContext){
    if(removeEventHandlerCallback)
    {
        removeEventHandlerCallback();
        removeEventHandlerCallback = nullptr;
    }

    systemContext = newContext;
    int id;
    switch (systemContext)
    {
    case SystemContext::EARLY_UPDATE:
        id = *GameUpdateEventHandler::earlyHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *GameUpdateEventHandler::earlyHandler -= id; };
        break;

    case SystemContext::UPDATE:
        id = *GameUpdateEventHandler::updateHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *GameUpdateEventHandler::updateHandler -= id; };
        break;

    case SystemContext::FIXED_UPDATE:
        id = *GameUpdateEventHandler::fixedUpdateHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *GameUpdateEventHandler::fixedUpdateHandler -= id; };
        break;

    case SystemContext::LATE_UPDATE:
        id = *GameUpdateEventHandler::lateHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *GameUpdateEventHandler::lateHandler -= id; };
        break;

    case SystemContext::PRE_RENDER:
        id = *PreRenderEventHandler::preRenderHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *PreRenderEventHandler::preRenderHandler -= id; };
        break;

    case SystemContext::POST_RENDER:
        id = *PreRenderEventHandler::postRenderHandler += [this](){ this->UpdateContext(); };
        removeEventHandlerCallback = [id](){ *PreRenderEventHandler::postRenderHandler -= id; };
        break;
    }
};