#pragma once
#include "../Engine/Core/Engine.h"
#include <memory>
#include <filesystem>

class EditorInterfaceGetters
{
public:
    static std::shared_ptr<Engine> engine;
    static bool sceneViewEnabled;
    static bool gameViewEnabled;
    static bool projectSettingsEnabled;
    static bool assetBrowserIsOpened;
    static std::filesystem::path currentProjectPath;
};