#pragma once
#include "Systems/EditorDrawBoxSystem.h"
#include "Systems/EditorDrawAnchorSystem.h"
#include <memory>

class GizmosController{
private:
    std::shared_ptr<EditorDrawBoxSystem> drawBoxSystem;
    std::shared_ptr<EditorDrawAnchorSystem> drawAnchorSystem;
public:
    GizmosController();
};
