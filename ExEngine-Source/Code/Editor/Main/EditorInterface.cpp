#include "EditorInterface.h"
#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/GameCore/Runtime/RuntimeEvent/GameEarlyUpdateEventHandler.h"
#include "../../Engine/GameCore/Runtime/RuntimeEvent/GameLateUpdateEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../Engine/Core/Input/InputEvents/InputEventHandler.h"
#include "../EditorEvents/EditorEarlyUpdateEventHandler.h"
#include "../../Engine/Logger/Logger.h"

EditorInterface::EditorInterface(){
    InitializeEditor();
    CreateEditorBase();
    *GameEarlyUpdateEventHandler::handler += [this](int value){ this->EarlyUpdate(); };
    *PreRenderEventHandler::handler += [this](int value){ this->PreRender(); };
    InputEventHandler::Create();
    *InputEventHandler::handler += [this](SDL_Event& sdlEvent){ ImGui_ImplSDL2_ProcessEvent(&sdlEvent); };
}; 

void EditorInterface::InitializeEditor(){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto imguiIo = ImGui::GetIO();
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  

    ImGui_ImplSDL2_InitForSDLRenderer(ExRendererGetters::window, ExRendererGetters::renderer);
    ImGui_ImplSDLRenderer2_Init(ExRendererGetters::renderer);
    
    EditorEarlyUpdateEventHandler::Create();
};

void EditorInterface::CreateEditorBase(){
    mainMenuBar = std::make_unique<ExEditor::MainMenuBar>();
};

void EditorInterface::EarlyUpdate() const{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();
};

void EditorInterface::PreRender() const{ //need to call on late update
    EditorEarlyUpdateEventHandler::handler->Invoke();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ExRendererGetters::renderer);
};

EditorInterface::~EditorInterface(){
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
};