#include "Time.h"
#include "../Settings/RuntimeSettings.h"
#include <SDL.h>

float Time::lastUpdate = 0;
float Time::deltaTime = 0;
float Time::lastFixedUpdate = 0;
float Time::fixedDeltaTime = 0;

bool Time::PermissionForUpdate(){
    if(RuntimeSettings::GetTargetFps() == -1)
        return true;

    auto ticks = SDL_GetTicks();

    auto timeSinceLastFrame = ticks - lastUpdate;
    auto timeSinceLastPermission = ticks - lastFixedUpdate;

    bool permission = timeSinceLastPermission > RuntimeSettings::GetTimePerFrame();

    if(permission)
    {
        lastFixedUpdate = ticks;
        fixedDeltaTime = timeSinceLastPermission / 1000;
    }

    lastUpdate = ticks;
    deltaTime = timeSinceLastFrame / 1000;

    return permission;
}