#include <SDL.h>
#include "Gameloop.h"
#include "../../Core/Runtime/Settings/RuntimeSettings.h"
#include "../../Core/Runtime/Time/Time.h"

void Gameloop::ExecuteGameLoop(){
    if(!isRunning) return;

    ProcessInputPhase();
    auto fixedUpdatePermission = Time::PermissionForUpdate();

    if(fixedUpdatePermission)
    {
        FixedUpdate();
    }

    ProcessCollisionPhase();
};

void Gameloop::Initialize(){
    isRunning = true;
    lastUpdate = 0;
};

void Gameloop::FixedUpdate(){
    
};

void Gameloop::Stop(){
    isRunning = false;
};

void Gameloop::ProcessInputPhase(){

};

void Gameloop::ProcessCollisionPhase(){

};