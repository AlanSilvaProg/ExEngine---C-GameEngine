#pragma once
#include "../EntityCounter/EntityCSCounter.h"
#include "../Component/IEComponentS.h"
#include "IPool.h"
#include <vector>
#include <memory>

class EComponentSPoolManager : public IPool{
private:
    std::vector<std::shared_ptr<IEComponentS>> componentInstanceByEntity; // entity id -> componentId
public:
    EComponentSPoolManager() = default;
    ~EComponentSPoolManager() = default;

    void ComponentAddedToEntity(const unsigned int entityId, std::shared_ptr<IEComponentS> component);
    void ComponentRemovedFromEntity(const unsigned int entityId);
    std::shared_ptr<IEComponentS> GetComponent(const unsigned int entityId) const;

    int GetInternalVectorSize() { return componentInstanceByEntity.size();};
};