#pragma once

class EntityCSCounter{
private:
    static unsigned int entitiesCreated;
public:    
    static unsigned int GetEntitiesCreated();
    static unsigned int IncreaseEntitiesCreated();
};