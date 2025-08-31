#pragma once
#include "EntityBrowserSelection.h"
#include "../../EditorWindow.h"
#include <imgui.h>
#include <memory>

class EntityBrowserWindow : public EditorWindow{
private:
    std::unique_ptr<EntityBrowserSelection> entityBrowserSelection;
public:
    EntityBrowserWindow();

    void Draw(int phase) override; //0 == early 1 == late
    void DrawEntity(int entityId);
};