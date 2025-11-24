#include "EditorInterface.h"
#include "EditorPresetInfo.h"
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../Engine/Core/Input/InputEvents/InputEventHandler.h"
#include "../../Engine/Core/Input/Input.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../../Engine/Core/Runtime/App.h" 
#include "../../Engine/Logger/Logger.h"
#include "../../Engine/File/FileManagement.h"
#include "../../Engine/Core/Runtime/Time/Time.h"
#include "EditorInterfaceGetters.h"
#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include <glm/glm.hpp>

EditorInterface::EditorInterface(std::shared_ptr<Engine> engine, std::string& gamePath){
    EditorInterfaceGetters::engine = engine;
    EditorInterfaceGetters::currentProjectPath = gamePath;

    Logger::Log("Editor initialized with the game located at: " + gamePath);
    
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
    ImGuiContext* gameContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(gameContext);
    ImGui::GetIO().IniFilename = "imgui_game.ini";
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
    ImGui_ImplSDL2_InitForSDLRenderer(ExRendererGetters::window, ExRendererGetters::renderer);
    ImGui_ImplSDLRenderer2_Init(ExRendererGetters::renderer);

    //Editor loading presets
    EditorPresetInfo result;
    if(FileManagement::LoadFromJson(std::string("engine_editor_layout"), result))
    {
        EditorInterfaceGetters::assetBrowserIsOpened = result.assetBrowserIsOpened;
        EditorInterfaceGetters::gameViewEnabled = result.gameViewEnabled;
        EditorInterfaceGetters::projectSettingsEnabled = result.projectSettingsEnabled;
        EditorInterfaceGetters::sceneViewEnabled = result.sceneViewEnabled;
        EditorInterfaceGetters::buildTarget = result.buildTarget;

        ImGui::LoadIniSettingsFromMemory(result.editorLayout.c_str());
    }

    EditorInterfaceGetters::defaultIconsInformation["DefaultIcons"] = std::make_unique<SpriteInformation>("Engine-Image-Icon", ICONS_PATH / "AssetIcons.png", glm::vec2(4,2));
    
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
    if(!App::isPlaying)
        Time::PermissionForUpdate();
};

void EditorInterface::LateUpdate() const{
    if(Input::GetButtonDown(SDLK_ESCAPE))
    {
        App::Quit();
    }

    SDL_Keymod mod = SDL_GetModState();

    if (((mod & KMOD_CTRL) || (mod & KMOD_GUI)) && Input::GetButtonDown(SDLK_s))
    {
        EditorInterfaceGetters::Save();
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
    //Saving Editor presets
    EditorPresetInfo result;
    result.assetBrowserIsOpened = EditorInterfaceGetters::assetBrowserIsOpened;
    result.gameViewEnabled = EditorInterfaceGetters::gameViewEnabled;
    result.projectSettingsEnabled = EditorInterfaceGetters::projectSettingsEnabled;
    result.sceneViewEnabled = EditorInterfaceGetters::sceneViewEnabled;
    result.buildTarget = EditorInterfaceGetters::buildTarget;
    //layout persistence
    size_t size;
    result.editorLayout = ImGui::SaveIniSettingsToMemory(&size);

    FileManagement::SaveFile(std::string("engine_editor_layout"), result.ToJson().dump());
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
};