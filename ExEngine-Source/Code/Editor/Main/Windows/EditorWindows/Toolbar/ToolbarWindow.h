#pragma once
#include "../../EditorWindow.h"

class ToolbarWindow : public EditorWindow{
public:
    void Draw(const int phase) override; //0 == early 1 == late
};
