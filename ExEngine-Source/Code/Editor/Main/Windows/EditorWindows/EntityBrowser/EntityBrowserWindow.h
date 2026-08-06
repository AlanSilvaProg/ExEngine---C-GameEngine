#pragma once
#include "EntityBrowserSelection.h"
#include "../../EditorWindow.h"
#include "../EngineConfig/WindowSizeManager.h"
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

class EntityBrowserWindow : public EditorWindow{
private:
    std::string const defaultEntityName = "New Entity";
    std::unique_ptr<EntityBrowserSelection> entityBrowserSelection;
    bool selectionDetected;

    void CheckContextWindowWithoutSelection();
    void NavigateSelectionWithArrows(const std::vector<int>& visibleEntities);
    void Duplicate();
    void Delete();
    bool IsValidSelection();
public:
    EntityBrowserWindow();

    void Draw(int phase) override; //0 == early 1 == late
    void DrawEntity(int entityId);
};