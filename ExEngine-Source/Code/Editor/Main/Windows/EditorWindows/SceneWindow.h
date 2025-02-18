#pragma once
#include "../EditorWindow.h"

class SceneWindow : public EditorWindow {
public: 
    void Draw(int phase) override; //0 == early 1 == late
};