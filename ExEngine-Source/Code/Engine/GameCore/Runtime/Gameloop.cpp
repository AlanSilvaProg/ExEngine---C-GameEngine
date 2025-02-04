#include <SDL.h>
#include "Gameloop.h"
#include "../../Core/Runtime/Settings/RuntimeSettings.h"
#include "../../Core/Runtime/Time/Time.h"
#include "../../Core/Rendering/Renderer/ExRenderer.h"
#include "../../Core/Input/Input.h"

void Gameloop::ExecuteGameLoop(){
    if(!isRunning) return;

    auto fixedUpdatePermission = Time::PermissionForUpdate();

    if(fixedUpdatePermission)
    {
        FixedUpdate();
    }
};

void Gameloop::Initialize(){
    isRunning = true;
    lastUpdate = 0;
};

void Gameloop::FixedUpdate(){
    GameEarlyUpdateEventHandler::handler->Invoke(0);

    ProcessInputPhase();
    ecsManager->Update();
    ProcessCollisionPhase();
    ProcessRenderPhase();   

    GameLateUpdateEventHandler::handler->Invoke(0);
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

};