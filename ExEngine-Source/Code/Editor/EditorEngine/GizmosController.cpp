#include "GizmosController.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/ECS/ECSManager.h"

GizmosController::GizmosController(){
    auto ecsmanager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    drawBoxSystem = ecsmanager->CreateSystem<EditorDrawBoxSystem>();
    drawAnchorSystem = ecsmanager->CreateSystem<EditorDrawAnchorSystem>();

     *EditorUpdateEventHandler::earlyHandler += [this](){ drawBoxSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
     *EditorUpdateEventHandler::earlyHandler += [this](){ drawAnchorSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
};
