#pragma once
#include <SDL.h>
#include <unordered_map>
#include <glm/glm.hpp>

class Input{
private:
    //Button Inputs map
    static std::unordered_map<SDL_Keycode, bool> keyMap;
    static std::unordered_map<SDL_Keycode, bool> lastStateKeyMap;

    //Mouse Inputs map
    static std::unordered_map<Uint8, bool> mouseButtonMap;
    static std::unordered_map<Uint8, bool> mouseLastStateButtonMap;

    static glm::ivec2 mousePosition;

public:
    //Button Inputs
    const static bool GetButtonPressed(SDL_KeyCode key);
    const static bool GetButtonDown(SDL_KeyCode key);
    const static bool GetButtonUp(SDL_KeyCode key);

    //Mouse Inputs
    const static bool GetMouseButtonPressed(Uint8 button);
    const static bool GetMouseButtonDown(Uint8 button);
    const static bool GetMouseButtonUp(Uint8 button);
    const static glm::ivec2 GetMousePosition();

    static void Process();
};