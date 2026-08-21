#include "Input.h"
#include "../../Logger/Logger.h"
#include "InputEvents/InputEventHandler.h"

std::unordered_map<SDL_Keycode, bool> Input::keyMap;
std::unordered_map<SDL_Keycode, bool> Input::lastStateKeyMap;
std::unordered_map<Uint8, bool> Input::mouseButtonMap;
std::unordered_map<Uint8, bool> Input::mouseLastStateButtonMap;
glm::ivec2 Input::mousePosition = glm::ivec2(0, 0);

const bool Input::GetButtonPressed(SDL_KeyCode key){
    return keyMap[key];
};

const bool Input::GetButtonDown(SDL_KeyCode key){
    return keyMap[key] && !lastStateKeyMap[key];
};

const bool Input::GetButtonUp(SDL_KeyCode key){
    return !keyMap[key] && lastStateKeyMap[key];
};

const bool Input::GetMouseButtonPressed(Uint8 button){
    return mouseButtonMap[button];
};

const bool Input::GetMouseButtonDown(Uint8 button){
    return mouseButtonMap[button] && !mouseLastStateButtonMap[button];
};

const bool Input::GetMouseButtonUp(Uint8 button){
    return !mouseButtonMap[button] && mouseLastStateButtonMap[button];
};

const glm::ivec2 Input::GetMousePosition(){
    return mousePosition;
};

void Input::Process(){
    SDL_Event sdlEvent;

    for(auto keyPair : keyMap){
        lastStateKeyMap[keyPair.first] = keyPair.second;
    }

    for(auto buttonPair : mouseButtonMap){
        mouseLastStateButtonMap[buttonPair.first] = buttonPair.second;
    }

    while(SDL_PollEvent(&sdlEvent)){

        InputEventHandler::handler->Invoke(sdlEvent);

        switch(sdlEvent.type)
        {
            case SDL_QUIT:
                keyMap[SDL_KeyCode::SDLK_ESCAPE] = true;
                break;
            case SDL_KEYUP:
                lastStateKeyMap[sdlEvent.key.keysym.sym] = keyMap[sdlEvent.key.keysym.sym];
                keyMap[sdlEvent.key.keysym.sym] = false;
                break;
            case SDL_KEYDOWN:
                lastStateKeyMap[sdlEvent.key.keysym.sym] = keyMap[sdlEvent.key.keysym.sym];
                keyMap[sdlEvent.key.keysym.sym] = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouseLastStateButtonMap[sdlEvent.button.button] = mouseButtonMap[sdlEvent.button.button];
                mouseButtonMap[sdlEvent.button.button] = true;
                break;
            case SDL_MOUSEBUTTONUP:
                mouseLastStateButtonMap[sdlEvent.button.button] = mouseButtonMap[sdlEvent.button.button];
                mouseButtonMap[sdlEvent.button.button] = false;
                break;
            case SDL_MOUSEMOTION:
                mousePosition = glm::ivec2(sdlEvent.motion.x, sdlEvent.motion.y);
                break;
        }
    }
};