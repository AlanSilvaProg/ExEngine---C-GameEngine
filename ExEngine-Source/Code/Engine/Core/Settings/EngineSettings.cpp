#include "EngineSettings.h"

std::string EngineSettings::engineName = "ExGameEngine";

std::string EngineSettings::GetEngineStringId(){
    return engineName; // ToDo include here the version
};