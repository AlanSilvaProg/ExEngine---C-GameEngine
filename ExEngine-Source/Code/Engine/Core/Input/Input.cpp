#include "Input.h"
#include "../../Logger/Logger.h"
#include "InputEvents/InputEventHandler.h"

std::unordered_map<SDL_Keycode, bool> Input::keyMap;
std::unordered_map<SDL_Keycode, bool> Input::lastStateKeyMap;

const bool Input::GetButtonPressed(SDL_KeyCode key){
    return keyMap[key];
};

const bool Input::GetButtonDown(SDL_KeyCode key){
    return keyMap[key] && !lastStateKeyMap[key];
};

const bool Input::GetButtonUp(SDL_KeyCode key){
    return !keyMap[key] && lastStateKeyMap[key];
};

void Input::Process(){
    SDL_Event sdlEvent;

    for(auto keyPair : keyMap){
        lastStateKeyMap[keyPair.first] = keyPair.second;
    }

    while(SDL_PollEvent(&sdlEvent)){

        InputEventHandler::handler->Invoke(sdlEvent);

        switch(sdlEvent.type)
        {
        case SDL_QUIT:
            keyMap[SDLK_ESCAPE] = true;
            break;
        case SDL_KEYUP:
            lastStateKeyMap[sdlEvent.key.keysym.sym] = keyMap[sdlEvent.key.keysym.sym];
            keyMap[sdlEvent.key.keysym.sym] = false;
            break;
        case SDL_KEYDOWN:
            lastStateKeyMap[sdlEvent.key.keysym.sym] = keyMap[sdlEvent.key.keysym.sym];
            keyMap[sdlEvent.key.keysym.sym] = true;
            break;
        }
    }
};