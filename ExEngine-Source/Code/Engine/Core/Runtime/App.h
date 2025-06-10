#pragma once
#include <string>

class App {
public:
    static bool isPlaying;
    static bool isDebugMode;
    static bool isEditorMode;

    App();
    static void Quit();
    static std::string GetCurrentProjectPath();
};