#include "EntityBrowserSelection.h"

void EntityBrowserSelection::SetEntitySelected(const int entityId){
    currentSelectedEntityId = entityId;
    Select();
};

const int EntityBrowserSelection::GetSelectedEntityId() const{
    return currentSelectedEntityId;
};