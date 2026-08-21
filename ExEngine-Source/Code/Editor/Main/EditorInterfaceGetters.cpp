#include "EditorInterfaceGetters.h"
#include "../../Engine/Core/Engine.h"
#include "../../Engine/Core/Scene/ECSWorldManager.h"
#include "../../Engine/Core/Runtime/App.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <filesystem>

std::shared_ptr<Engine> EditorInterfaceGetters::engine = nullptr;
std::shared_ptr<ScriptHotReloadManager> EditorInterfaceGetters::scriptHotReloadManager = nullptr;
bool EditorInterfaceGetters::sceneViewEnabled = false;
bool EditorInterfaceGetters::gameViewEnabled = false;
bool EditorInterfaceGetters::consoleEnabled = false;
bool EditorInterfaceGetters::projectSettingsEnabled = false;
bool EditorInterfaceGetters::ecsMonitoringEnabled = false;
bool EditorInterfaceGetters::ecsAdministratorEnabled = false;
bool EditorInterfaceGetters::assetBrowserIsOpened = false;
bool EditorInterfaceGetters::engineConfigEnabled = false;
std::filesystem::path EditorInterfaceGetters::currentProjectPath;
std::filesystem::path EditorInterfaceGetters::currentWorldPath;
BuildType EditorInterfaceGetters::buildTarget;

std::map<std::string, std::unique_ptr<SpriteInformation>> EditorInterfaceGetters::defaultIconsInformation;

bool EditorInterfaceGetters::worldWithoutPath;
bool EditorInterfaceGetters::buildWindowEnabled;

void EditorInterfaceGetters::Save(){
    if(App::isPlaying) return;

    if(EditorInterfaceGetters::worldWithoutPath)
    {
        std::filesystem::path targetPath = EditorInterfaceGetters::currentProjectPath/"Assets/Worlds";

        if(!std::filesystem::exists(targetPath))
            std::filesystem::create_directory(targetPath);

        const char* folder = tinyfd_saveFileDialog(
            "Save world at",
            (targetPath/"DefaultWorld.exworld").c_str(), 0
            , nullptr, nullptr
        );

        if(folder == nullptr) 
        {
            Logger::LogError("Save World failed!");
            return;
        }

        auto& currentWorldPath = ECSWorldManager::GetCurrentWorld()->GetWorldPath();
        currentWorldPath = std::filesystem::path(folder);
        worldWithoutPath = false;
    }

    ECSWorldManager::SaveCurrentWorld();
};

void EditorInterfaceGetters::Reload(){
    ECSWorldManager::ReloadCurrentWorldState();
};