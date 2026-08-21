#pragma once
#include "Windows/EditorWindows/BuildSettings/BuildType.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/AssetManager/SpriteInformation.h"
#include "../Scripting/ScriptHotReloadManager.h"
#include <memory>
#include <filesystem>
#include <map>

class EditorInterfaceGetters
{
public:
    static std::shared_ptr<Engine> engine;
    static std::shared_ptr<ScriptHotReloadManager> scriptHotReloadManager;
    static bool sceneViewEnabled;
    static bool gameViewEnabled;
    static bool projectSettingsEnabled;
    static bool consoleEnabled;
    static bool ecsMonitoringEnabled;
    static bool ecsAdministratorEnabled;
    static bool assetBrowserIsOpened;
    static bool engineConfigEnabled;
    static std::filesystem::path currentProjectPath;
    static std::filesystem::path currentWorldPath;
    static BuildType buildTarget;

    static inline std::filesystem::path GetAssetsPath() { return currentProjectPath / "Assets"; };

    //don't need to be saved
    static bool worldWithoutPath;
    static bool buildWindowEnabled;

    static std::map<std::string, std::unique_ptr<SpriteInformation>> defaultIconsInformation;

    static void Save();
    static void Reload();
};