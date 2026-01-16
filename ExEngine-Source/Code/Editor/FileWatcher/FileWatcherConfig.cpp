#include "FileWatcherConfig.h"
#include "../../Engine/Logger/Logger.h"
#include "../../Engine/File/FileManagement.h"

const std::string FileWatcherConfig::CONFIG_VERSION = "1.0";

bool FileWatcherConfig::SaveConfig(const WatchConfig& config, const std::string& filePath) {
    try {
        std::string configKey = filePath.empty() ? "filewatcher_config" : filePath;
        
        // Create a copy to add version info
        WatchConfig configWithVersion = config;
        
        // Use FileManagement to save
        if (FileManagement::SaveFile(configKey, const_cast<WatchConfig&>(configWithVersion))) {
            return true;
        } else {
            Logger::LogError("FileWatcher: Failed to save configuration to: " + configKey);
            return false;
        }
        
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Error saving configuration: " + std::string(e.what()));
        return false;
    }
}

bool FileWatcherConfig::LoadConfig(WatchConfig& config, const std::string& filePath) {
    try {
        std::string configKey = filePath.empty() ? "filewatcher_config" : filePath;
        
        // Use FileManagement to load
        if (FileManagement::LoadFromJson(configKey, config)) {
            return true;
        } else {
            return false;
        }
        
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Error loading configuration: " + std::string(e.what()));
        return false;
    }
}

std::string FileWatcherConfig::GetDefaultConfigPath() {
    // Return a simple key name for FileManagement system
    return "filewatcher_config";
}