#include "EditorWindowDrawer.h"
#include "../../EditorEvents/EditorUpdateEventHandler.h"
#include "EditorWindows/SceneView/SceneWindow.h"
#include "EditorWindows/GameView/GameWindow.h"
#include "EditorWindows/EntityBrowser/EntityBrowserWindow.h"
#include "EditorWindows/Inspector/ExInspectorWindow.h"
#include "EditorWindows/ProjectSettings/ExProjectSettingsWindow.h"
#include "EditorWindows/AssetBrowser/AssetBrowserWindow.h"

std::vector<std::shared_ptr<EditorWindow>> ExEditor::EditorWindowDrawer::windows;

ExEditor::EditorWindowDrawer::EditorWindowDrawer(){
    AddWindow(std::make_shared<SceneWindow>());
    AddWindow(std::make_shared<GameWindow>());
    AddWindow(std::make_shared<EntityBrowserWindow>());
    AddWindow(std::make_shared<ExInspectorWindow>());
    AddWindow(std::make_shared<ExProjectSettingsWindow>());
    AddWindow(std::make_shared<AssetBrowserWindow>());

    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Draw(0); };
    *EditorUpdateEventHandler::lateHandler += [this](){ this->Draw(1); };
};

ExEditor::EditorWindowDrawer::~EditorWindowDrawer(){
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