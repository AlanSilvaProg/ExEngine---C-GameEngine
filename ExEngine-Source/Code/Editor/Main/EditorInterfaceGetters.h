#pragma once
#include <memory>
#include "../Engine/Core/Engine.h"

class EditorInterfaceGetters
{
public:
    static std::shared_ptr<Engine> engine;
    static bool sceneViewEnabled;
    static bool gameViewEnabled;
    static bool projectSettingsEnabled;
};