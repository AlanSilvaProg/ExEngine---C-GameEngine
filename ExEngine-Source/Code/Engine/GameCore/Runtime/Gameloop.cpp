#include "Gameloop.h"
#include "../../Core/Runtime/Settings/RuntimeSettings.h"
#include "../../Core/Runtime/Time/Time.h"
#include "../../Core/Rendering/Renderer/ExRenderer.h"
#include "../../Core/Input/Input.h"
#include "../../Core/Runtime/App.h"
#include "../../Core/CollisionSystem/ExPhysicsEngine.h"
#include <SDL.h>

void Gameloop::ExecuteGameLoop(){
    if(!isRunning) return;

    auto fixedUpdatePermission = Time::PermissionForUpdate();

    GameUpdateEventHandler::earlyHandler->Invoke();

    if(fixedUpdatePermission)
        FixedUpdate();
    else
    {
        Update();
    }  

    GameUpdateEventHandler::lateHandler->Invoke();
};

void Gameloop::Initialize(){
    isRunning = true;
};

void Gameloop::Update(){
    ProcessInputPhase();
    ecsManager->Update();
    ProcessCollisionPhase();

    GameUpdateEventHandler::updateHandler->Invoke();

#ifdef EXENGINE_EDITOR
    ProcessRenderPhase();
#endif
};

void Gameloop::FixedUpdate(){
    Update();

    GameUpdateEventHandler::fixedUpdateHandler->Invoke();

#ifndef EXENGINE_EDITOR
    ProcessRenderPhase();
#endif
};

void Gameloop::Stop(){
    isRunning = false;
};

void Gameloop::ProcessRenderPhase(){
    ExRenderer::RenderSequence();
};

void Gameloop::ProcessInputPhase(){
    Input::Process();
};

void Gameloop::ProcessCollisionPhase(){
    if(!App::isPlaying) return;

    ExPhysicsEngine::RunCollisionRoutine();
};