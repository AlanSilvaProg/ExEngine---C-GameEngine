#pragma once
#include "ConfigurationFileProperties.h"
#include <memory>
#include <filesystem>

#ifndef CONFIGURATION_FILE_NAME
#define CONFIGURATION_FILE_NAME "ExConfiguration"
#endif

class ConfigurationFileManager{
private:
    static std::unique_ptr<ConfigurationFileProperties> configurationFileProperties;
    static std::filesystem::path fullFilePath;
    
public:
    static bool Load();
    static void SaveCurrentState();
};