#pragma once
#include <SDL.h>
#include <unordered_map>

class Input{
private:
    static std::unordered_map<SDL_Keycode, bool> keyMap;
    static std::unordered_map<SDL_Keycode, bool> lastStateKeyMap;

public:
    const static bool GetButtonPressed(SDL_KeyCode key);
    const static bool GetButtonDown(SDL_KeyCode key);
    const static bool GetButtonUp(SDL_KeyCode key);

    static void Process();
};