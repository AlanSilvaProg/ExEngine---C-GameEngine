#include "GizmosController.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/ECS/ECSManager.h"

GizmosController::GizmosController(){
    auto ecsmanager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    drawBoxSystem = ecsmanager->CreateSystem<EditorDrawBoxSystem>();
    drawAnchorSystem = ecsmanager->CreateSystem<EditorDrawAnchorSystem>();
    drawCameraSafeAreaSystem = ecsmanager->CreateSystem<EditorDrawCameraSafeAreaSystem>();
    drawTextLabelSystem = ecsmanager->CreateSystem<EditorDrawTextLabelSystem>();

    // Drawn on the late handler (fired from PreRenderEventHandler::postRenderHandler, after
    // ExRenderer::RenderQueue() has drawn every sprite/label) so gizmos land on top of the scene
    // instead of being overdrawn by it - no background clear happens in between.
     *EditorUpdateEventHandler::lateHandler += [this](){ drawBoxSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
     *EditorUpdateEventHandler::lateHandler += [this](){ drawAnchorSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
     *EditorUpdateEventHandler::lateHandler += [this](){ drawCameraSafeAreaSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
     *EditorUpdateEventHandler::lateHandler += [this](){ drawTextLabelSystem->UpdateSystem(SystemContext::EARLY_UPDATE); };
};
