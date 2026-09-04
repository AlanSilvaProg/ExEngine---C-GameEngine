#pragma once

enum EditorSelectableType{
    Entity = 0,
    Asset = 1
};

class IEditorSelectable{
public:
    virtual EditorSelectableType GetType() = 0;  
    void Select();
};

class ElementSelectionController{
private:
    static IEditorSelectable* currentSelection;
    static bool deleteCommandOverridden;
public:
    static void SetSelected(IEditorSelectable* selectable);
    static IEditorSelectable* GetCurrentSelection();

    // Lets a window with its own sub-selection (e.g. a selected animation keyframe) claim
    // priority over the shared Delete command, so deleting that sub-selection doesn't also
    // delete the entity that owns it just because the entity is still the main selection.
    static void SetDeleteCommandOverridden(const bool overridden);
    static bool IsDeleteCommandOverridden();
};