#pragma once 
#include <string>

class EngineSettings {
private:
    static std::string engineName;
public:
    static std::string GetEngineStringId();
};