#pragma once
#include <vector>
#include <memory>

template<typename TComponent>
class EComponentSPoolManager : public IPool{
private:
    std::vector<std::shared_ptr<TComponent>> componentInstanceByEntity; // entity id -> componentId
public:
    EComponentSPoolManager() = default;
    ~EComponentSPoolManager() = default;

    void ComponentAddedToEntity(const unsigned int entityId, std::shared_ptr<TComponent> component){
        if(componentInstanceByEntity.size() <= entityId)
            componentInstanceByEntity.resize(entityId * 2, nullptr);
        
        componentInstanceByEntity[entityId] = component;
    };

    std::shared_ptr<TComponent> GetComponent(const unsigned int entityId){
        return componentInstanceByEntity[entityId];
    };
};