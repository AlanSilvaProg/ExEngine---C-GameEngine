#include "EntityCSCounter.h"

unsigned int EntityCSCounter::entitiesCreated = 0;

unsigned int EntityCSCounter::GetEntitiesCreated(){ return entitiesCreated; };

unsigned int EntityCSCounter::IncreaseEntitiesCreated() { 
    entitiesCreated++;
    return GetEntitiesCreated();
};