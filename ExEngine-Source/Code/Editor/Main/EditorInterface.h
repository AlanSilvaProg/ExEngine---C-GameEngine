#pragma once 
#include <memory>
#include "MainMenuBar/MainMenuBar.h"
#include "Windows/EditorWindowDrawer.h"
#include "../Engine/Core/Engine.h"

class EditorInterface{
private:
    std::unique_ptr<ExEditor::MainMenuBar> mainMenuBar;
    std::unique_ptr<ExEditor::EditorWindowDrawer> editorWindowDrawer;

    void InitializeEditor();
    void EarlyUpdate() const;
    void LateUpdate() const;
    void PreRender() const;
    void PostRender() const;
    void PostRenderPresent() const;

    void CreateEditorBase();
public:
    EditorInterface(std::shared_ptr<Engine> engine);
    ~EditorInterface();
};