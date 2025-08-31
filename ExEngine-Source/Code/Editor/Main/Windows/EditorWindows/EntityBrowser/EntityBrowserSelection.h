#pragma once
#include "../ElementSelectionController.h"

class EntityBrowserSelection: public IEditorSelectable{
private:
    int currentSelectedEntityId;
public:
    inline virtual EditorSelectableType GetType() override { return EditorSelectableType::Entity; };
    void SetEntitySelected(const int entityId);
    const int GetSelectedEntityId() const;
};