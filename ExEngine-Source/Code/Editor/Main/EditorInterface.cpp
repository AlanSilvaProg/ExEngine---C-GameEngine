#include "EditorInterface.h"
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../Engine/Core/Input/InputEvents/InputEventHandler.h"
#include "../../Engine/Core/Input/Input.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/Runtime/App.h" 
#include "../../Engine/Logger/Logger.h"
#include "EditorInterfaceGetters.h"
#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
#include <SDL.h>

EditorInterface::EditorInterface(std::shared_ptr<Engine> engine){
    EditorInterfaceGetters::engine = engine;
    
    InitializeEditor();
    CreateEditorBase();
    InputEventHandler::Create();
    *InputEventHandler::handler += [this](SDL_Event& sdlEvent){ ImGui_ImplSDL2_ProcessEvent(&sdlEvent); };
    *GameUpdateEventHandler::earlyhandler += [this](int value){ this->EarlyUpdate(); };
    *GameUpdateEventHandler::latehandler += [this](int value){ this->LateUpdate(); };
    *PreRenderEventHandler::preRenderHandler += [this](){ this->PreRender(); };
    *PreRenderEventHandler::postRenderHandler += [this](){ this->PostRender(); };
}; 

void EditorInterface::InitializeEditor(){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
    ImGui_ImplSDL2_InitForSDLRenderer(ExRendererGetters::window, ExRendererGetters::renderer);
    ImGui_ImplSDLRenderer2_Init(ExRendererGetters::renderer);
    
    EditorUpdateEventHandler::Create();
};

void EditorInterface::CreateEditorBase(){
    mainMenuBar = std::make_unique<ExEditor::MainMenuBar>();
    editorWindowDrawer = std::make_unique<ExEditor::EditorWindowDrawer>();
};

void EditorInterface::EarlyUpdate() const{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    ImGui::ShowDemoWindow();
};

void EditorInterface::LateUpdate() const{
    //ToDo: Implement a X button to close the editor
    if(Input::GetButtonDown(SDLK_ESCAPE))
    {
        App::Quit();
    }
};

void EditorInterface::PreRender() const{ //need to call on late update
    EditorUpdateEventHandler::earlyHandler->Invoke();
};

void EditorInterface::PostRender() const{ //need to call on late update
    EditorUpdateEventHandler::lateHandler->Invoke();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ExRendererGetters::renderer);
};

EditorInterface::~EditorInterface(){
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
};