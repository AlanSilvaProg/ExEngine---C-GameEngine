#include "App.h"
#include "AppEvents/AppEventsHandler.h"

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

void App::Quit(){
    AppEventsHandler::onApplicationQuitHandler->Invoke();
};