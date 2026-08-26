#include <SDL.h>
#include "RuntimeSettings.h"

const int MILLI = 1000;
int RuntimeSettings::targetFramesPerSeconds = 120; // -1 to unlimited fps
float RuntimeSettings::timePerFrame = 0;
std::string RuntimeSettings::externalTextEditorPath = "";
bool RuntimeSettings::physicsEnabled = true;
bool RuntimeSettings::drawAllPhysics = true;
bool RuntimeSettings::autoSaveOnPlay = false;

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

void RuntimeSettings::SetPhysicsEnabled(bool enabled){
    physicsEnabled = enabled;
};

bool RuntimeSettings::GetPhysicsEnabled(){
    return physicsEnabled;
};

void RuntimeSettings::SetDrawAllPhysics(bool enabled){
    drawAllPhysics = enabled;
};

bool RuntimeSettings::GetDrawAllPhysics(){
    return drawAllPhysics;
};

void RuntimeSettings::SetAutoSaveOnPlay(bool enabled){
    autoSaveOnPlay = enabled;
};

bool RuntimeSettings::GetAutoSaveOnPlay(){
    return autoSaveOnPlay;
};