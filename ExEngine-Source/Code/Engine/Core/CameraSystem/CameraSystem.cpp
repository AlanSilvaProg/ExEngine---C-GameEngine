#include "CameraSystem.h"
#include "../Components/CameraComponent.h"
#include "../Rendering/Renderer/ExRendererGetters.h"
#include "../Components/TransformComponent.h"
#include <SDL.h>

CameraSystem::CameraSystem(){
    Require<CameraComponent>(false);
};

void CameraSystem::UpdateSystem(){
    
};