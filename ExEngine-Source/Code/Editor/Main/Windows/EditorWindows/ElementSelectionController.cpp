#include "ElementSelectionController.h"

IEditorSelectable* ElementSelectionController::currentSelection = nullptr;

void ElementSelectionController::SetSelected(IEditorSelectable* selectable){
    currentSelection = selectable;
};

IEditorSelectable* ElementSelectionController::GetCurrentSelection(){
    return currentSelection;
};

void IEditorSelectable::Select(){
    ElementSelectionController::SetSelected(this);
};