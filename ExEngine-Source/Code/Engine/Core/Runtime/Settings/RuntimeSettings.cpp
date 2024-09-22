#include "RuntimeSettings.h"

int RuntimeSettings::targetFramesPerSeconds = 60;
float RuntimeSettings::timePerFrame = 0;

void RuntimeSettings::SetTargetFps(int targetFps){
    targetFramesPerSeconds = targetFps;
    timePerFrame = static_cast<float>(targetFramesPerSeconds) / 1000;
};

int RuntimeSettings::GetTargetFps(){
    return targetFramesPerSeconds;
};  

float RuntimeSettings::GetTimePerFrame(){
    if(timePerFrame == 0)
        SetTargetFps(targetFramesPerSeconds);
    return timePerFrame;
};