#pragma once
#include "Windows/EditorWindows/BuildSettings/BuildType.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/AssetManager/SpriteInformation.h"
#include <memory>
#include <filesystem>
#include <map>

class EditorInterfaceGetters
{
public:
    static std::shared_ptr<Engine> engine;
    static bool sceneViewEnabled;
    static bool gameViewEnabled;
    static bool projectSettingsEnabled;
    static bool assetBrowserIsOpened;
    static std::filesystem::path currentProjectPath;
    static BuildType buildTarget;

    //don't need to be saved
    static bool worldWithoutPath;
    static bool buildWindowEnabled;

    static std::map<std::string, std::unique_ptr<SpriteInformation>> defaultIconsInformation;

    static void Save();
    static void Reload();
};