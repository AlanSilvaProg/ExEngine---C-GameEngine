#include "EditorInterface.h"
#include "EditorPresetInfo.h"
#include "EditorInterfaceGetters.h"
#include "Windows/EditorWindows/EngineConfig/ConfigurationManager.h"
#include "Windows/EditorWindows/EngineConfig/EngineConfigWindow.h"
#include "../Scripting/ScriptCompiler.h"
#include "../EditorEvents/EditorUpdateEventHandler.h"
#include "../EditorEvents/EditorCommandEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../Engine/GameCore/Runtime/RuntimeEvent/GameUpdateEventHandler.h"
#include "../../Engine/Core/Rendering/Renderer/RendererEvent/PreRenderEventHandler.h"
#include "../../Engine/Core/CameraSystem/NoCameraEventHandler.h"
#include "../../Engine/Core/Input/InputEvents/InputEventHandler.h"
#include "../../Engine/Core/Input/Input.h"
#include "../../Engine/Core/Scene/ECSWorldManager.h"
#include "../../Engine/Core/Settings/EngineSettings.h"
#include "../../Engine/Core/Runtime/App.h"
#include "../../Engine/Logger/Logger.h"
#include "../../Engine/File/FileManagement.h"
#include "../../Engine/Core/Runtime/Time/Time.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
#include <SDL.h>
#include <glm/glm.hpp>

EditorInterface::EditorInterface(std::shared_ptr<Engine> engine, std::string& gamePath){
    EditorInterfaceGetters::engine = engine;
    EditorInterfaceGetters::currentProjectPath = gamePath;

    auto assetsPath = EditorInterfaceGetters::GetAssetsPath();
    if(!std::filesystem::exists(assetsPath))
    {
        std::filesystem::create_directories(assetsPath);
    }

    // Lets an external editor opened on a project script (under Assets/) resolve engine headers.
    ScriptCompiler::WriteIntelliSenseConfig(EditorInterfaceGetters::currentProjectPath);

    Logger::Log("Editor initialized with the game located at: " + gamePath);
    
    InitializeEditor();
    InitializeScriptHotReload();
    InitializeFileWatcher();
    CreateEditorBase();
    InputEventHandler::Create();
    *InputEventHandler::handler += [this](SDL_Event& sdlEvent){ ImGui_ImplSDL2_ProcessEvent(&sdlEvent); };
    *GameUpdateEventHandler::earlyHandler += [this](){ this->EarlyUpdate(); };
    *GameUpdateEventHandler::lateHandler += [this](){ this->LateUpdate(); };
    *PreRenderEventHandler::preRenderHandler += [this](){ this->PreRender(); };
    *PreRenderEventHandler::postRenderHandler += [this](){ this->PostRender(); };
    *NoCameraEventHandler::noCameraHandler += [this](){ this->DrawNoCameraOverlay(); };
};

void EditorInterface::InitializeEditor(){
    IMGUI_CHECKVERSION();
    ImGuiContext* gameContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(gameContext);
    ImGui::GetIO().IniFilename = "imgui_game.ini";
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;
    ImGui_ImplSDL2_InitForSDLRenderer(ExRendererGetters::window, ExRendererGetters::renderer);
    ImGui_ImplSDLRenderer2_Init(ExRendererGetters::renderer);

    ConfigurationManager::Initialize();

    auto path = EditorInterfaceGetters::currentProjectPath;
    path.append(EDITOR_LAYOUT_FILE_NAME);

    //Editor loading presets - Project based file first then try loading engine based file.
    EditorPresetInfo result;
    if(FileManagement::LoadFromJson(path, result) || FileManagement::LoadFromJson(EDITOR_LAYOUT_FILE_NAME, result))
    {
        EditorInterfaceGetters::inspectorEnabled = result.inspectorEnabled;
        EditorInterfaceGetters::entityBrowserEnabled = result.entityBrowserEnabled;
        EditorInterfaceGetters::projectSettingsEnabled = result.projectSettingsEnabled;
        EditorInterfaceGetters::consoleEnabled = result.consoleEnabled;
        EditorInterfaceGetters::ecsMonitoringEnabled = result.ecsMonitoringEnabled;
        EditorInterfaceGetters::ecsAdministratorEnabled = result.ecsAdministratorEnabled;
        EditorInterfaceGetters::assetBrowserIsOpened = result.assetBrowserIsOpened;
        EditorInterfaceGetters::currentWorldPath = result.currentWorldPath;
        EditorInterfaceGetters::engineConfigEnabled = result.engineConfigEnabled;
        EditorInterfaceGetters::buildTarget = result.buildTarget;

        ImGui::LoadIniSettingsFromMemory(result.editorLayout.c_str());
    }
    
    ConfigurationManager::LoadStyleConfig();

    EditorInterfaceGetters::defaultIconsInformation["DefaultIcons"] = std::make_unique<SpriteInformation>("Engine-Image-Icon", ICONS_PATH / "AssetIcons.png", glm::vec2(4,2));
    EditorInterfaceGetters::defaultIconsInformation["PlayIcon"] = std::make_unique<SpriteInformation>("Engine-Play-Icon", ICONS_PATH / "botao-play-ponta-de-seta.png", glm::vec2(1,1));
    EditorInterfaceGetters::defaultIconsInformation["PauseIcon"] = std::make_unique<SpriteInformation>("Engine-Pause-Icon", ICONS_PATH / "pausa.png", glm::vec2(1,1));
    
    EditorUpdateEventHandler::Create();
    EditorCommandEventHandler::Create();
};

void EditorInterface::CreateEditorBase(){
    mainMenuBar = std::make_unique<ExEditor::MainMenuBar>();
    editorWindowDrawer = std::make_unique<ExEditor::EditorWindowDrawer>();
    editorCameraController = std::make_unique<EditorCameraController>();
    editorSelectionController = std::make_unique<EditorSelectionController>();
    gizmosController = std::make_unique<GizmosController>();
};

void EditorInterface::EarlyUpdate() const{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowDemoWindow();
    if(!App::isPlaying)
        Time::PermissionForUpdate();

    if(scriptHotReloadManager) scriptHotReloadManager->Poll();

    UpdateWindowTitle();
    ClampWindowsToViewport();
};

// Keeps floating editor windows reachable after the OS window is resized/shrunk:
// their last known position (from the previous frame or a loaded layout) can end up
// outside the new display bounds, so we pull them back in every frame.
void EditorInterface::ClampWindowsToViewport() const{
    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    if(displaySize.x <= 0.0f || displaySize.y <= 0.0f) return;

    constexpr float minVisible = 40.0f; // minimum sliver of the window kept on screen

    for(ImGuiWindow* window : ImGui::GetCurrentContext()->Windows)
    {
        if(!window->WasActive) continue;
        if(window->ParentWindow != nullptr) continue; // child/popup/docked windows follow their parent
        if(window->Flags & ImGuiWindowFlags_ChildWindow) continue;

        const ImVec2 clampedPos(
            ImClamp(window->Pos.x, minVisible - window->Size.x, displaySize.x - minVisible),
            ImClamp(window->Pos.y, 0.0f, displaySize.y - minVisible)
        );

        if(clampedPos.x != window->Pos.x || clampedPos.y != window->Pos.y)
            ImGui::SetWindowPos(window, clampedPos);
    }
};

void EditorInterface::UpdateWindowTitle() const{
    std::string sceneName = "Unsaved scene";
    if(ECSWorldManager::HasCurrentWorld() && !EditorInterfaceGetters::worldWithoutPath)
        sceneName = ECSWorldManager::GetCurrentWorldInfo().name;

    std::string title = EngineSettings::GetEngineStringId() + " - " + sceneName;
    SDL_SetWindowTitle(ExRendererGetters::window, title.c_str());
};

void EditorInterface::LateUpdate() const{
    if(Input::GetButtonDown(SDLK_ESCAPE))
    {
        App::Quit();
    }

    SDL_Keymod mod = SDL_GetModState();

    if (((mod & KMOD_CTRL) || (mod & KMOD_GUI)))
    {
        if(Input::GetButtonDown(SDLK_s))
            EditorInterfaceGetters::Save();
        if(Input::GetButtonDown(SDLK_d))
            EditorCommandEventHandler::duplicate->Invoke();
        if(Input::GetButtonDown(SDLK_BACKSPACE))
            EditorCommandEventHandler::deleteCmmd->Invoke();
    }

    if(mod == KMOD_NONE)
    {
        if(Input::GetButtonDown(SDLK_f))
            EditorCommandEventHandler::focusSelected->Invoke();
    }

    // Handle global keyboard shortcuts for Engine Config window
    EngineConfigWindow::HandleGlobalKeyboardShortcuts();
};

void EditorInterface::PreRender() const{
    EditorUpdateEventHandler::earlyHandler->Invoke();
};

void EditorInterface::DrawNoCameraOverlay() const{
    // Scene View always has the editor's own camera; the "no game camera" state only
    // matters while looking through the game's cameras (Game View).
    if(EditorInterfaceGetters::viewMode == EditorViewMode::SceneView) return;

    SDL_SetRenderDrawColor(ExRendererGetters::renderer, 0, 0, 0, 255);
    SDL_RenderClear(ExRendererGetters::renderer);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(ExRendererGetters::window, &windowWidth, &windowHeight);

    const char* message = "No camera has been created yet";
    const ImVec2 textSize = ImGui::CalcTextSize(message);
    const ImVec2 textPos((windowWidth - textSize.x) * 0.5f, (windowHeight - textSize.y) * 0.5f);

    ImGui::GetBackgroundDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), message);
};

void EditorInterface::PostRender() const{
    EditorUpdateEventHandler::lateHandler->Invoke();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), ExRendererGetters::renderer);
    SDL_RenderPresent(ExRendererGetters::renderer);
};

EditorInterface::~EditorInterface(){
    // Stop file watcher before shutdown
    if (fileWatcher) {
        fileWatcher->SaveConfiguration(); // Save current configuration
        fileWatcher->Stop();
    }
    
    // Save engine configuration (including style settings) before shutdown
    ConfigurationManager::SaveStyleConfig();
    
    //Saving Editor presets
    EditorPresetInfo result;
    result.inspectorEnabled = EditorInterfaceGetters::inspectorEnabled;
    result.entityBrowserEnabled = EditorInterfaceGetters::entityBrowserEnabled;
    result.projectSettingsEnabled = EditorInterfaceGetters::projectSettingsEnabled;
    result.consoleEnabled = EditorInterfaceGetters::consoleEnabled;
    result.ecsMonitoringEnabled = EditorInterfaceGetters::ecsMonitoringEnabled;
    result.ecsAdministratorEnabled = EditorInterfaceGetters::ecsAdministratorEnabled;
    result.assetBrowserIsOpened = EditorInterfaceGetters::assetBrowserIsOpened;
    result.engineConfigEnabled = EditorInterfaceGetters::engineConfigEnabled;
    result.buildTarget = EditorInterfaceGetters::buildTarget;

    if(!EditorInterfaceGetters::worldWithoutPath){
        result.currentWorldPath = ECSWorldManager::GetCurrentWorld()->GetWorldPath();
    }

    //layout persistence
    size_t size;
    result.editorLayout = ImGui::SaveIniSettingsToMemory(&size);

    auto path = EditorInterfaceGetters::currentProjectPath;
    path.append(EDITOR_LAYOUT_FILE_NAME);

    FileManagement::SaveFile(path, result.ToJson().dump());
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
};

void EditorInterface::InitializeScriptHotReload(){
    auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    scriptHotReloadManager = std::make_shared<ScriptHotReloadManager>(ecsManager, EditorInterfaceGetters::currentProjectPath);
    EditorInterfaceGetters::scriptHotReloadManager = scriptHotReloadManager;
    scriptHotReloadManager->ScanAndCompileExistingScripts();
};

void EditorInterface::InitializeFileWatcher() {
    fileWatcher = std::make_unique<FileWatcher>();
    
    // Load configuration if it exists
    fileWatcher->LoadConfiguration();
    
    // Set up default watch paths if none are configured
    if (!EditorInterfaceGetters::currentProjectPath.empty()) {
        std::filesystem::path projectPath(EditorInterfaceGetters::currentProjectPath);
        
        // Watch the project directory
        fileWatcher->AddWatchPath(projectPath.string(), true);
        
        // Watch common asset directories
        std::filesystem::path assetsPath = projectPath / "Assets";
        if (std::filesystem::exists(assetsPath)) {
            fileWatcher->AddWatchPath(assetsPath.string(), true);
        }
        
        std::filesystem::path scriptsPath = projectPath / "Scripts";
        if (std::filesystem::exists(scriptsPath)) {
            fileWatcher->AddWatchPath(scriptsPath.string(), true);
        }
    }
    
    // Set up file filters for common asset types
    std::vector<std::string> assetExtensions = {
        ".png", ".jpg", ".jpeg", ".bmp", ".tga",  // Images
        ".wav", ".mp3", ".ogg",                   // Audio
        ".obj", ".fbx", ".dae",                   // 3D Models
        ".cpp", ".h", ".hpp",                     // Scripts
        ".json", ".xml", ".txt"                   // Data files
    };
    fileWatcher->SetFileFilter(assetExtensions);
    
    // Exclude temporary and system files
    std::vector<std::string> excludePatterns = {
        ".*\\.tmp$",        // Temporary files
        ".*\\.temp$",       // Temporary files
        ".*~$",             // Backup files
        ".*\\.DS_Store$",   // macOS system files
        ".*Thumbs\\.db$"    // Windows thumbnail cache
    };
    fileWatcher->SetExcludePatterns(excludePatterns);
    
    // Register event callbacks
    fileWatcher->OnFileCreated += [this](const FileEvent& event) {
        this->OnFileCreated(event);
    };
    
    fileWatcher->OnFileChanged += [this](const FileEvent& event) {
        this->OnFileModified(event);
    };
    
    fileWatcher->OnFileDeleted += [this](const FileEvent& event) {
        this->OnFileDeleted(event);
    };
    
    // Start monitoring
    if (!fileWatcher->Start()) {
        Logger::LogError("Failed to start FileWatcher");
    } else {
        Logger::Log("FileWatcher started successfully");
    }
}

void EditorInterface::OnFileCreated(const FileEvent& event) {
    Logger::Log("File created: " + event.filePath);

    if (event.GetExtension() == ".hpp" && scriptHotReloadManager) {
        scriptHotReloadManager->OnScriptFileEvent(event.filePath);
    }

    // Notify asset browser to refresh
    // This would trigger a refresh of the asset browser window
    // The actual implementation would depend on how the asset browser is structured
}

void EditorInterface::OnFileModified(const FileEvent& event) {
    Logger::Log("File modified: " + event.filePath);

    // Handle different file types
    std::string extension = event.GetExtension();

    if (extension == ".hpp") {
        if (scriptHotReloadManager) scriptHotReloadManager->OnScriptFileEvent(event.filePath);
    } else if (extension == ".h" || extension == ".cpp") {
        Logger::Log("Script file modified, consider reloading: " + event.filePath);
    } else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
        // Image file changed - might need to reload texture
        Logger::Log("Image file modified, consider reloading texture: " + event.filePath);
    }
}

void EditorInterface::OnFileDeleted(const FileEvent& event) {
    Logger::Log("File deleted: " + event.filePath);

    if (event.GetExtension() == ".hpp" && scriptHotReloadManager) {
        scriptHotReloadManager->OnScriptFileDeleted(event.filePath);
    }

    // Notify asset browser to refresh and remove references
    // Clean up any loaded resources that reference this file
}