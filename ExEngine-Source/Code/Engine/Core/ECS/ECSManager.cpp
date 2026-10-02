#include "ECSManager.h"
#include "../../Logger/Logger.h"
#include "../Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "../Serializer/Demangle.h"
#include "../Serializer/ExSerializedFieldSetter.h"
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

    //ToDo Reload Contexts Content
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

std::shared_ptr<EntityCS> ECSManager::CreateEntity(const std::string entityName){
    if(freeEntities.empty()){
        auto entity = std::make_shared<EntityCS>(EntityCSCounter::GetEntitiesCreated(), entityName, this);
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

void ECSManager::DuplicateEntity(const int entityId){
    DuplicateEntityInternal(entityId, -1);
};

std::shared_ptr<EntityCS> ECSManager::DuplicateEntityInternal(const int entityId, const int forcedParentId){
    auto entityToDuplicate = GetEntity(entityId);

    if(entityToDuplicate == nullptr) return nullptr;

    auto entityToDuplicateSignature = GetEntitySignature(entityId);

    auto entity = CreateEntity(entityToDuplicate->GetName() + "_duplicate");
    auto newEntityId = entity->GetId();
    auto& newEntitySignature = GetEntitySignature(newEntityId);

    //duplicating signatures
    newEntitySignature.resize(entityToDuplicateSignature.size());

    for(auto i = 0; i < newEntitySignature.size(); i++)
    {
        newEntitySignature[i] = entityToDuplicateSignature[i];
    }

    //duplicating component content
    for(auto componentId = 0; componentId < newEntitySignature.size(); componentId++)
    {
        if(newEntitySignature[componentId])
        {
            componentPools[componentId]->CopyComponent(entityId, newEntityId);
        }
    }

    // Root of the duplication keeps the original's parent (becomes a sibling); duplicated
    // descendants are re-parented under their own duplicated parent instead.
    const int parentForNewEntity = forcedParentId >= 0 ? forcedParentId : entityToDuplicate->parentId;
    if(parentForNewEntity >= 0){
        SetParent(parentForNewEntity, newEntityId);
    }

    auto childrenSnapshot = entityToDuplicate->childrenId;
    for(auto childId : childrenSnapshot){
        DuplicateEntityInternal(childId, newEntityId);
    }

    return entity;
};

bool ECSManager::WouldCreateCycle(const unsigned int parentId, const unsigned int childrenId) const{
    if(parentId == childrenId) return true;

    // Walking up from the prospective parent: if we ever reach childrenId, parenting childrenId
    // under parentId would make childrenId its own ancestor.
    int currentId = static_cast<int>(parentId);
    while(currentId >= 0){
        if(static_cast<unsigned int>(currentId) == childrenId) return true;

        auto current = entities[currentId];
        if(current == nullptr) break;

        currentId = current->parentId;
    }

    return false;
};

bool ECSManager::SetParent(const unsigned int parentId, const unsigned int childrenId){
    auto parent = GetEntity(parentId);
    if(parent == nullptr) return false;
    auto children = GetEntity(childrenId);
    if(children == nullptr) return false;

    if(WouldCreateCycle(parentId, childrenId)){
        Logger::LogError("Cannot set parent " + std::to_string(parentId) + " for entity " + std::to_string(childrenId) + ": would create a cycle in the entity hierarchy.");
        return false;
    }

    auto childrenParentId = children->parentId;
    if(childrenParentId >= 0){
        RemoveChildren(childrenParentId, childrenId);
    }

    parent->SetChildren(childrenId);
    children->SetParent(parentId);

    return true;
};

bool ECSManager::RemoveChildren(const unsigned int parentId, const unsigned int childrenId){
    auto parent = GetEntity(parentId);
    if(parent == nullptr) return false;

    if(parent->IsChildren(childrenId)){
        parent->RemoveChildren(childrenId);
    }

    auto children = GetEntity(childrenId);
    if(children == nullptr) return true;

    if(children->IsParent(parentId)){
        children->RemoveParent();
    }

    return true;
};

bool ECSManager::RemoveParent(const unsigned int entityId){
    auto entity = GetEntity(entityId);
    if(entity == nullptr){
        Logger::LogError("Error when trying to remove parent from a unexistent entity id: " + std::to_string(entityId));
        return false;
    }

    entity->RemoveParent();
    return true;
};

void ECSManager::DestroyEntityImmediately(int entityId){
    if(!aliveEntities.contains(entityId)) return;

    auto entity = entities[entityId];

    // Snapshot first - destroying a child mutates this same vector out from under the loop.
    auto childrenSnapshot = entity->childrenId;
    for(auto childId : childrenSnapshot){
        DestroyEntityImmediately(childId);
    }

    if(entity->parentId >= 0){
        RemoveChildren(entity->parentId, entityId);
    }
    entity->childrenId.clear();
    entity->enabled = true;

    RemoveAllComponents(entities[entityId]);
    aliveEntities.erase(entityId);
    freeEntities.push_back(entityId);

    Logger::Log("Entity with ID: " + std::to_string(entityId) + " has been killed.");
};

void ECSManager::DestroyEntity(const int entity){
    entitiesToBeKilled.emplace_back(entity);
};

void ECSManager::DestroyEntity(const std::shared_ptr<EntityCS> entity){
    DestroyEntity(entity->GetId());
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

void ECSManager::UpdateEntityComponentsByJson(const int id, const nlohmann::json& componentsContent){
    //ToDo update entity components based on json
};

bool ECSManager::ApplyComponentUpdate(const int entityId, const ComponentUpdate& update){
    if(!HasComponent(entityId, update.componentId)) return false;

    auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentPools[update.componentId]);
    if(castedPoolManager == nullptr) return false;

    auto component = castedPoolManager->GetComponent(entityId);
    if(component == nullptr) return false;

    auto serializedComponent = component->Serialize();
    for(const auto& field : serializedComponent.serializedFields)
    {
        if(field.fieldName != update.fieldName) continue;
        return ExSerializedFieldSetter::TrySetValueFromJson(field, update.newValue);
    }

    return false;
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

void ECSManager::DestroySystem(const std::type_index typeIndex){
    systems.erase(typeIndex);
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

const std::string EntityCS::GetName() const{
    return name;
};

void EntityCS::SetParent(const unsigned int parentId){
    this->parentId = parentId;
};

void EntityCS::SetChildren(const unsigned int childrenId){
    if(IsChildren(childrenId)) return;

    this->childrenId.push_back(childrenId);
};

const unsigned int EntityCS::GetParentId() const{
    return parentId;
};

std::shared_ptr<EntityCS> EntityCS::GetParent() const{
    if(parentId < 0) return nullptr;
    return ecsManager->GetEntity(parentId);
};

const std::vector<unsigned int>& EntityCS::GetChildrens() const{
    return childrenId;
};

void EntityCS::RemoveParent(){
    parentId = -1;
};

bool EntityCS::RemoveChildren(const unsigned int childrenId){
    auto it = std::find(this->childrenId.begin(), this->childrenId.end(), childrenId);

    if (it == this->childrenId.end())
        return false;

    this->childrenId.erase(it);
    return true;
};

bool EntityCS::IsChildren(const unsigned int entityId) const{
    return std::find(this->childrenId.begin(), this->childrenId.end(), entityId) != this->childrenId.end();
};

bool EntityCS::IsParent(const unsigned int entityId) const{
    return parentId == entityId;
};

bool EntityCS::SetEnabled(const bool value){
    if(value){
        auto parent = GetParent();
        if(parent != nullptr && !parent->IsEnabled()){
            Logger::LogWarning("Cannot enable entity '" + name + "' while its parent is inactive.");
            return false;
        }
    }

    enabled = value;
    ecsManager->SetToValidation(id);

    for(auto childId : childrenId){
        auto child = ecsManager->GetEntity(childId);
        if(child != nullptr) child->SetEnabled(value);
    }

    return true;
};

void EntityCS::UpdateComponentsByJson(const nlohmann::json& componentsContent){
    ecsManager->UpdateEntityComponentsByJson(id, componentsContent);
};

Signature& EntityCS::GetComponentSignature() const{
    return ecsManager->GetEntitySignature(GetId());
};

void EntityCS::RemoveComponent(const int componentId) const{
    ecsManager->RemoveComponent(GetId(), componentId);
};

void EntityCS::ApplyComponentUpdate(const ComponentUpdate& update) const{
    ecsManager->ApplyComponentUpdate(GetId(), update);
};

void EntityCS::KillImmediately(){
    ecsManager->DestroyEntityImmediately(id);
};

void EntityCS::Kill(){
    ecsManager->DestroyEntity(ecsManager->GetEntity(id));
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
    const bool eligible = entity->IsEnabled() && CheckEntitySignatureMatch(entity->GetComponentSignature());

    for(int i = 0; i < systemEntities.size(); i++)
    {
        if(systemEntities[i]->GetId() == entityId)
        {
            if(!eligible)
            {
                RemoveEntity(entityId);
            }
            return;
        }
    }

    if(eligible)
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

void ECSystem::RemoveRequirement(const int componentId){
    if(!CheckForRegisteredId(componentId)) return;
    
    std::erase(systemOptionalSignatureIds, componentId);
    std::erase(systemSignatureIds, componentId);
};

bool ECSystem::CheckForRegisteredId(const int componentId) const{
    for(auto id : systemOptionalSignatureIds)
    {
        if(id == componentId)
        {
            Logger::LogWarning("Component has already been registered as optional: " + Demangle(typeid(*this).name()));
            return true;
        }
    }

    for(auto id : systemSignatureIds)
    {
        if(id == componentId)
        {
            Logger::LogWarning("Component has already been registered as requirement: " + Demangle(typeid(*this).name()));
            return true;
        }
    }
    return false;
};

// System Context

void ECSystemContext::UpdateContext(){
    if(!enabled) return;

    for(auto& systemEntry : systemEntries)
    {
        systemEntry.system->UpdateSystem(systemContext);
    }

    for(auto& systemEntry : customSystemEntries)
    {
        systemEntry->UpdateSystem(systemContext);
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