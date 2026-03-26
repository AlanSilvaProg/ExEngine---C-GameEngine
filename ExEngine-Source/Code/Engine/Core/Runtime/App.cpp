#include "App.h"
#include "AppEvents/AppEventsHandler.h"
#include <fstream>
#include <filesystem>

#ifdef GAME_BUILD
bool App::isPlaying = true;
#else
bool App::isPlaying = false;
#endif
bool App::isDebugMode = false;
bool App::isEditorMode = false;

App::App(){
    AppEventsHandler::Create();
};

std::string App::GetCurrentProjectPath(){
    return std::filesystem::current_path();
};

void App::Quit(){
    AppEventsHandler::onApplicationQuitHandler->Invoke();
};