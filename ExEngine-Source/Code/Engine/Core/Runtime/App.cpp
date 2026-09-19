#include "App.h"
#include "AppEvents/AppEventsHandler.h"
#include <fstream>
#include <filesystem>
#include <curl/curl.h>
#include "../../Logger/Logger.h"

#ifdef GAME_BUILD
bool App::isPlaying = true;
#else
bool App::isPlaying = false;
#endif
bool App::isDebugMode = false;
bool App::isEditorMode = false;

App::App(){
    AppEventsHandler::Create();
    auto result = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(result != CURLE_OK){
        Logger::LogError("Curl initialization failed with CURLcode: " + std::to_string(result));
    }
};

std::string App::GetCurrentProjectPath(){
    return std::filesystem::current_path();
};

void App::Quit(){
    AppEventsHandler::onApplicationQuitHandler->Invoke();
    curl_global_cleanup();
};