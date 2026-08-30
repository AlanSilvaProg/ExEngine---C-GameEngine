#include "GizmosController.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/ECS/ECSManager.h"

GizmosController::GizmosController(){
    auto ecsmanager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    drawBoxSystem = ecsmanager->CreateSystem<EditorDrawBoxSystem>();

     *EditorUpdateEventHandler::earlyHandler += [this](){ drawBoxSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
};
