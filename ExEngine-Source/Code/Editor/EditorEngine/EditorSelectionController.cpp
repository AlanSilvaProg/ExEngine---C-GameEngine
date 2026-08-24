#include "EditorSelectionController.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/ECS/ECSManager.h"

EditorSelectionController::EditorSelectionController(){
    auto ecsmanager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    clickSystem = ecsmanager->CreateSystem<EditorClickSystem>();

    *EditorUpdateEventHandler::earlyHandler += [this](){ clickSystem->UpdateSystem(); };
};
