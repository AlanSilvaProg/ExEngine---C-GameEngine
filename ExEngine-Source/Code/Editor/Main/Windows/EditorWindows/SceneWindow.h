#pragma once
#include "../EditorWindow.h"
#include <imgui.h>

class SceneWindow : public EditorWindow {
private:
    ImVec2 lastMousePos;
public: 
    SceneWindow() : lastMousePos(0,0){};

    void Draw(int phase) override; //0 == early 1 == late
};