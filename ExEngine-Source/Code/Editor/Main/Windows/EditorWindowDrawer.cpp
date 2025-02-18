#include "EditorWindowDrawer.h"
#include "../../EditorEvents/EditorUpdateEventHandler.h"
#include "EditorWindows/SceneWindow.h"

std::vector<std::shared_ptr<EditorWindow>> ExEditor::EditorWindowDrawer::windows;

ExEditor::EditorWindowDrawer::EditorWindowDrawer(){
    AddWindow(std::make_shared<SceneWindow>());
    *EditorUpdateEventHandler::earlyHandler += [this](){ this->Draw(0); };
    *EditorUpdateEventHandler::lateHandler += [this](){ this->Draw(1); };
    *EditorUpdateEventHandler::postRenderPresentHandler  += [this](){ this->Draw(2); };
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