#pragma once 
#include "MainMenuBar/MainMenuBar.h"
#include <memory>

class EditorInterface{
private:
    std::unique_ptr<ExEditor::MainMenuBar> mainMenuBar;

    void InitializeEditor();
    void EarlyUpdate() const;
    void PreRender() const;

    void CreateEditorBase();
public:
    EditorInterface();
    ~EditorInterface();
};