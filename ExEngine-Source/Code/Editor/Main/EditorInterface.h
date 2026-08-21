#pragma once
#include "MainMenuBar/MainMenuBar.h"
#include "Windows/EditorWindowDrawer.h"
#include "../Engine/Core/Engine.h"
#include "../FileWatcher/FileWatcher.h"
#include "../Scripting/ScriptHotReloadManager.h"
#include <memory>
#include <filesystem>

#ifndef ICONS_PATH
#define ICONS_PATH EditorInterfaceGetters::engine->GetEngineAssetsPath() / "Editor"
#endif

#ifndef EDITOR_LAYOUT_FILE_NAME
#define EDITOR_LAYOUT_FILE_NAME std::string("engine_editor_layout")
#endif

class EditorInterface{
private:
    std::unique_ptr<ExEditor::MainMenuBar> mainMenuBar;
    std::unique_ptr<ExEditor::EditorWindowDrawer> editorWindowDrawer;
    std::unique_ptr<FileWatcher> fileWatcher;
    std::shared_ptr<ScriptHotReloadManager> scriptHotReloadManager;

    void InitializeEditor();
    void InitializeFileWatcher();
    void InitializeScriptHotReload();
    void OnFileCreated(const FileEvent& event);
    void OnFileModified(const FileEvent& event);
    void OnFileDeleted(const FileEvent& event);
    void EarlyUpdate() const;
    void LateUpdate() const;
    void PreRender() const;
    void PostRender() const;
    void UpdateWindowTitle() const;

    void CreateEditorBase();
public:
    EditorInterface(std::shared_ptr<Engine> engine, std::string& gamePath);
    ~EditorInterface();
};