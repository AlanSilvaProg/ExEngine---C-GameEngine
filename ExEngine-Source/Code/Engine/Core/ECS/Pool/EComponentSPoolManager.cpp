#include "EComponentSPoolManager.h"

std::shared_ptr<IEComponentS> EComponentSPoolManager::GetComponent(const unsigned int entityId) const{
    if(entityId >= componentInstanceByEntity.size()) return nullptr;
    
    return componentInstanceByEntity[entityId];
};

void EComponentSPoolManager::ComponentAddedToEntity(const unsigned int entityId, std::shared_ptr<IEComponentS> component){
    if(componentInstanceByEntity.size() <= entityId)
        componentInstanceByEntity.resize(EntityCSCounter::GetEntitiesCreated() * 2, nullptr);
  
    componentInstanceByEntity[entityId] = component;
};

void EComponentSPoolManager::ComponentRemovedFromEntity(const unsigned int entityId)
{
    if(componentInstanceByEntity.size() <= entityId) return;
    componentInstanceByEntity[entityId] = nullptr;
};