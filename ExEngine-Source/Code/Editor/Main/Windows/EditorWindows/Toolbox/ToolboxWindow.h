#pragma once
#include "../../EditorWindow.h"

class ToolboxWindow : public EditorWindow{
private:
    bool collapsed = false;
    float lastKnownColumnWidth = 0.0f;

    void DrawWindowButtons();
    void DrawCollapseToggleBar(float width);
public:
    void Draw(const int phase) override; //0 == early 1 == late
};
