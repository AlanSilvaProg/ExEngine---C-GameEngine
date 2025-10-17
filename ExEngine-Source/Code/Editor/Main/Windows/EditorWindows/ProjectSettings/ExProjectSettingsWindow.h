#pragma once
#include "../../EditorWindow.h"

class ExProjectSettingsWindow: public EditorWindow{
public:
    ExProjectSettingsWindow() = default;
    void Draw(int phase) override; //0 == early 1 == late
};