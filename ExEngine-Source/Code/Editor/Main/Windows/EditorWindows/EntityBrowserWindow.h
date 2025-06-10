#pragma once
#include "EntityBrowserNode.h"
#include "../EditorWindow.h"
#include <imgui.h>

class EntityBrowserWindow : public EditorWindow{
private:
    unsigned int currentEntityId;
public:
    void Draw(int phase) override; //0 == early 1 == late
    void DrawEntity(int entityId);
};