#include <SDL.h>
#include "RuntimeSettings.h"

const int MILLI = 1000;
int RuntimeSettings::targetFramesPerSeconds = 120; // -1 to unlimited fps
float RuntimeSettings::timePerFrame = 0;
std::string RuntimeSettings::externalTextEditorPath = "";

void RuntimeSettings::SetTargetFps(int targetFps){
    targetFramesPerSeconds = targetFps;
    timePerFrame = MILLI / static_cast<float>(targetFramesPerSeconds);
};

int RuntimeSettings::GetTargetFps(){
    return targetFramesPerSeconds;
};  

float RuntimeSettings::GetTimePerFrame(){
    if(timePerFrame == 0)
        SetTargetFps(targetFramesPerSeconds);
    return timePerFrame;
};

void RuntimeSettings::SetExternalTextEditorPath(const std::string& path){
    externalTextEditorPath = path;
};

const std::string& RuntimeSettings::GetExternalTextEditorPath(){
    return externalTextEditorPath;
};