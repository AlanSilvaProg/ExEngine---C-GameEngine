#pragma once
#include "Systems/EditorDrawBoxSystem.h"
#include <memory>

class GizmosController{
private:
    std::shared_ptr<EditorDrawBoxSystem> drawBoxSystem;
public:
    GizmosController();
};
