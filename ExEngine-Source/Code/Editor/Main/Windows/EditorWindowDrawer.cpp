#include "EditorWindowDrawer.h"
#include "../../EditorEvents/EditorUpdateEventHandler.h"
#include "EditorWindows/Toolbar/ToolbarWindow.h"
#include "EditorWindows/EntityBrowser/EntityBrowserWindow.h"
#include "EditorWindows/Inspector/ExInspectorWindow.h"
#include "EditorWindows/ProjectSettings/ExProjectSettingsWindow.h"
#include "EditorWindows/AssetBrowser/AssetBrowserWindow.h"
#include "EditorWindows/BuildSettings/BuildSettingsWindow.h"
#include "EditorWindows/Console/ConsoleWindow.h"
#include "EditorWindows/ECSMonitoring/ECSMonitoring.h"
#include "EditorWindows/ECSAdmin/ECSAdmin.h"
#include "EditorWindows/EngineConfig/EngineConfigWindow.h"
#include "EditorWindows/ProcessProgress/ProcessProgressWindow.h"
#include "EditorWindows/GameShortcuts/GameShortcutsWindow.h"

std::vector<std::shared_ptr<EditorWindow>> ExEditor::EditorWindowDrawer::windows;

ExEditor::EditorWindowDrawer::EditorWindowDrawer(){
    AddWindow(std::make_shared<ToolbarWindow>());
    AddWindow(std::make_shared<EntityBrowserWindow>());
    AddWindow(std::make_shared<ExInspectorWindow>());
    AddWindow(std::make_shared<ExProjectSettingsWindow>());
    AddWindow(std::make_shared<AssetBrowserWindow>());
    AddWindow(std::make_shared<BuildSettingsWindow>());
    AddWindow(std::make_shared<ConsoleWindow>());
    AddWindow(std::make_shared<ECSMonitoring>());
    AddWindow(std::make_shared<ECSAdmin>());
    AddWindow(std::make_shared<EngineConfigWindow>());
    AddWindow(std::make_shared<ProcessProgressWindow>());
    AddWindow(std::make_shared<GameShortcutsWindow>());

    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Draw(0); };
    *EditorUpdateEventHandler::lateHandler += [this](){ this->Draw(1); };
};

ExEditor::EditorWindowDrawer::~EditorWindowDrawer(){
    for (auto& window : windows) {
        window.reset();
    }
    windows.clear();
};

void ExEditor::EditorWindowDrawer::Draw(int phase){
    for(auto window : windows){
        window->Draw(phase);
    }
};

void ExEditor::EditorWindowDrawer::AddWindow(std::shared_ptr<EditorWindow> window){
    for(auto w : windows){
        if(w == window)
            return;
    }

    windows.push_back(window);
};

void ExEditor::EditorWindowDrawer::RemoveWindow(std::shared_ptr<EditorWindow> window){
    windows.erase(std::remove_if(windows.begin(), windows.end(), [window](auto w){ return w == window; }), windows.end());
};