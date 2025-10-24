#include "ConfigurationFileManager.h"
#include "../../File/FileManagement.h"
#include "../../Logger/Logger.h"
#include "../../JsonUtility/JsonUtility.h"

std::unique_ptr<ConfigurationFileProperties> ConfigurationFileManager::configurationFileProperties = nullptr;
std::filesystem::path ConfigurationFileManager::fullFilePath = std::filesystem::path(ENGINE_CONFIG_PATH) / CONFIGURATION_FILE_NAME;

bool ConfigurationFileManager::Load(){
    if(configurationFileProperties == nullptr) configurationFileProperties = std::make_unique<ConfigurationFileProperties>();

    auto& result = *configurationFileProperties.get();
    auto loadResult = FileManagement::LoadFromJson(fullFilePath, result);

    if(loadResult)
    {
        Logger::Log("Configuration file was Loaded successfully");
        return true;
    }
    
    Logger::Log("Configuration file wasn't loaded successfully, a default version should be created");
    return false;
};

void ConfigurationFileManager::SaveCurrentState(){
    if(configurationFileProperties == nullptr) configurationFileProperties = std::make_unique<ConfigurationFileProperties>();

    auto stringfiedJson = JsonUtility::ToJson(*configurationFileProperties.get());
    FileManagement::SaveFile(fullFilePath, nlohmann::to_string(stringfiedJson));
};