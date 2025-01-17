#pragma once
#include <vector>
#include <memory>
#include "../EntityCounter/EntityCSCounter.h"

template<typename TComponent>
class EComponentSPoolManager : public IPool{
private:
    std::vector<std::shared_ptr<TComponent>> componentInstanceByEntity; // entity id -> componentId
public:
    EComponentSPoolManager() = default;
    ~EComponentSPoolManager() = default;

    void ComponentAddedToEntity(const unsigned int entityId, std::shared_ptr<TComponent> component);
    std::shared_ptr<TComponent> GetComponent(const unsigned int entityId);
};

template<typename TComponent>
std::shared_ptr<TComponent> EComponentSPoolManager<TComponent>::GetComponent(const unsigned int entityId){
    return componentInstanceByEntity[entityId];
};

template<typename TComponent>
void EComponentSPoolManager<TComponent>::ComponentAddedToEntity(const unsigned int entityId, std::shared_ptr<TComponent> component){
    if(componentInstanceByEntity.size() <= entityId)
        componentInstanceByEntity.resize(EntityCSCounter::GetEntitiesCreated() * 2, nullptr);
  
    componentInstanceByEntity[entityId] = component;
};