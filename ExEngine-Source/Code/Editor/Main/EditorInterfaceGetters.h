#pragma once
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

    static std::map<std::string, std::unique_ptr<SpriteInformation>> defaultIconsInformation;
};