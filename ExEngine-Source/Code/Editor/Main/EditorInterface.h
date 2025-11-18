#pragma once 
#include "MainMenuBar/MainMenuBar.h"
#include "Windows/EditorWindowDrawer.h"
#include "../Engine/Core/Engine.h"
#include <memory>
#include <filesystem>

#ifndef ICONS_PATH
#define ICONS_PATH EditorInterfaceGetters::engine->GetEngineAssetsPath() / "Editor"
#endif

class EditorInterface{
private:
    std::unique_ptr<ExEditor::MainMenuBar> mainMenuBar;
    std::unique_ptr<ExEditor::EditorWindowDrawer> editorWindowDrawer;

    void InitializeEditor();
    void EarlyUpdate() const;
    void LateUpdate() const;
    void PreRender() const;
    void PostRender() const;

    void CreateEditorBase();
public:
    EditorInterface(std::shared_ptr<Engine> engine, std::string& gamePath);
    ~EditorInterface();
};