#pragma once
#include "Systems/EditorClickSystem.h"
#include <memory>

class EditorSelectionController{
private:
    std::shared_ptr<EditorClickSystem> clickSystem;
public:
    EditorSelectionController();
};
