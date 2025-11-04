#include "ConfigurationFileManager.h"
#include "../Engine.h"
#include "../../File/FileManagement.h"
#include "../../Logger/Logger.h"
#include "../../JsonUtility/JsonUtility.h"

std::unique_ptr<ConfigurationFileProperties> ConfigurationFileManager::configurationFileProperties = nullptr;

bool ConfigurationFileManager::Load(){
    if(configurationFileProperties == nullptr) configurationFileProperties = std::make_unique<ConfigurationFileProperties>();

    auto& result = *configurationFileProperties.get();
    auto fullFilePath = Engine::GetEnginePath() / CONFIGURATION_FILE_NAME;
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
    auto fullFilePath = Engine::GetEnginePath() / CONFIGURATION_FILE_NAME;
    FileManagement::SaveFile(fullFilePath, nlohmann::to_string(stringfiedJson));
};