#pragma once
#include "../../EditorWindow.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "../../../../../Engine/Core/Engine.h"
#include "../../../../../Engine/Core/ECS/ECSManager.h"
#include <memory>

class ECSAdmin: public EditorWindow{
private:
    std::shared_ptr<ECSManager> ecsManager;

    bool showWorldEntities;
    bool showSystems;
    bool includeInternals;
    bool createNewECSystemTriggered;
    
    bool creatingSystem;
    char systemName[256];
    SystemContext selectedContext;

    bool showRenameDialog;
    char renameBuffer[256];
    std::shared_ptr<CustomECSystem> systemToRename;
    
    void DrawSystemWithContextMenu(const std::type_index* systemTypeId, std::shared_ptr<ECSystem> ecsystem, SystemContext currentContext);
    void DrawColumnElement(const SystemContext currentContext);
    void DrawCreateButton(const SystemContext currentContext);
    void DrawMoveToOption(const SystemContext currentContext, const SystemContext targetContext, const std::type_index* systemTypeId, std::shared_ptr<ECSystem> ecsystem);
    void DrawRenameDialog();
    void DrawCreateSystemDialog();
public:
    ECSAdmin();

    void Draw(int phase) override; //0 == early 1 == late
};