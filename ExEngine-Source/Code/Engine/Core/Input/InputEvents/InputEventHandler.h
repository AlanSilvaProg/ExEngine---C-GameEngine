#pragma once
#include "../../EventSystem/Event.h"
#include <SDL.h>

class InputEventHandler : public Event<SDL_Event&>{
public:
    static std::unique_ptr<InputEventHandler> handler;

    static void Create();
};