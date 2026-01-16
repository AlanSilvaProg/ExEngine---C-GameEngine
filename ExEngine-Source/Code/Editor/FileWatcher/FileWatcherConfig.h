#pragma once
#include "WatchConfig.h"
#include <string>

class FileWatcherConfig {
public:
    static bool SaveConfig(const WatchConfig& config, const std::string& filePath);
    static bool LoadConfig(WatchConfig& config, const std::string& filePath);
    static std::string GetDefaultConfigPath();
    
private:
    static const std::string CONFIG_VERSION;
};