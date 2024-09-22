#include <SDL.h>
#include "Gameloop.h"
#include "Settings/RuntimeSettings.h"
#include "Time/Time.h"

void Gameloop::ExecuteGameLoop(){
    if(!isRunning) return;

    ProcessInputPhase();

    if(Time::PermissionForUpdate())
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