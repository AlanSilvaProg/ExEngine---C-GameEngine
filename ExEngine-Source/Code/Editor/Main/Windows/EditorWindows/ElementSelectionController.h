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
public:
    static void SetSelected(IEditorSelectable* selectable);
    static IEditorSelectable* GetCurrentSelection();
};