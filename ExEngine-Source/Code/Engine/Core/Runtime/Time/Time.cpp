#include <SDL.h>
#include "Time.h"
#include "../Settings/RuntimeSettings.h"

float Time::lastUpdate = 0;
float Time::deltaTime = 0;

bool Time::PermissionForUpdate(){
    float timeSinceLastPermission = SDL_GetTicks() - lastUpdate;
    bool permission = timeSinceLastPermission > RuntimeSettings::GetTimePerFrame();

    lastUpdate = SDL_GetTicks();
    if(permission)
    {
        deltaTime = timeSinceLastPermission;
    }

    return permission;
}
