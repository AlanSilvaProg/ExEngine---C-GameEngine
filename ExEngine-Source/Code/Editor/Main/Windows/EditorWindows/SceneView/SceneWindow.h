#pragma once
#include "../../EditorWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "../../../../EditorECS/System/EditorCameraSystem.h"
#include <SDL.h>
#include <imgui.h>
#include <memory>

class SceneWindow : public EditorWindow {
private:
    std::shared_ptr<ECSManager> ecsManager;
    std::shared_ptr<EditorCameraSystem> editorCameraSystem;

    ImVec2 lastMousePos;
public: 
    static SDL_Texture* sceneDisplay;

    SceneWindow();

    void Draw(int phase) override; //0 == early 1 == late
};