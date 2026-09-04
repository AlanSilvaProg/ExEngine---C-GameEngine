#include "ElementSelectionController.h"

IEditorSelectable* ElementSelectionController::currentSelection = nullptr;
bool ElementSelectionController::deleteCommandOverridden = false;

void ElementSelectionController::SetSelected(IEditorSelectable* selectable){
    currentSelection = selectable;
};

IEditorSelectable* ElementSelectionController::GetCurrentSelection(){
    return currentSelection;
};

void ElementSelectionController::SetDeleteCommandOverridden(const bool overridden){
    deleteCommandOverridden = overridden;
};

bool ElementSelectionController::IsDeleteCommandOverridden(){
    return deleteCommandOverridden;
};

void IEditorSelectable::Select(){
    ElementSelectionController::SetSelected(this);
};