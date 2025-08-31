#pragma once
#include "../../EditorWindow.h"
#include <imgui.h>

class GameWindow : public EditorWindow {
private:
    static int targetDisplay;
public: 
    GameWindow();
    ~GameWindow() = default;

    void Draw(int phase) override; //0 == early 1 == late
};