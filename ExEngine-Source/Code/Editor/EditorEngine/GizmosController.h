#pragma once
#include "Systems/EditorDrawBoxSystem.h"
#include "Systems/EditorDrawAnchorSystem.h"
#include "Systems/EditorDrawCameraSafeAreaSystem.h"
#include "Systems/EditorDrawTextLabelSystem.h"
#include <memory>

class GizmosController{
private:
    std::shared_ptr<EditorDrawBoxSystem> drawBoxSystem;
    std::shared_ptr<EditorDrawAnchorSystem> drawAnchorSystem;
    std::shared_ptr<EditorDrawCameraSafeAreaSystem> drawCameraSafeAreaSystem;
    std::shared_ptr<EditorDrawTextLabelSystem> drawTextLabelSystem;
public:
    GizmosController();
};
