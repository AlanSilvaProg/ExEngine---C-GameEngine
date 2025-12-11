#include "ConfigurationManager.h"
#include "../../../../../Engine/Core/Engine.h"
#include "../../../../../Engine/File/FileManagement.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/JsonUtility/JsonUtility.h"
#include <filesystem>
#include <algorithm>
#include <fstream>

std::unique_ptr<StyleConfiguration> ConfigurationManager::currentStyle = nullptr;
std::string ConfigurationManager::configFilePath = "";
bool ConfigurationManager::initialized = false;
ConfigurationError ConfigurationManager::lastError;

void ConfigurationManager::Initialize() {
    if (initialized) return;
    
    if (currentStyle == nullptr) {
        currentStyle = std::make_unique<StyleConfiguration>();
    }
    
    auto enginePath = Engine::GetEnginePath();
    configFilePath = (enginePath / ENGINE_CONFIG_FILE_NAME).string();
    
    LoadStyleConfig();
    
    initialized = true;
}

StyleConfiguration& ConfigurationManager::GetStyleConfig() {
    if (!initialized) {
        Initialize();
    }
    
    if (currentStyle == nullptr) {
        currentStyle = std::make_unique<StyleConfiguration>();
    }
    
    return *currentStyle;
}

bool ConfigurationManager::SaveStyleConfig() {
    ClearError();
    
    if (!initialized) {
        Initialize();
    }
    
    if (currentStyle == nullptr) {
        SetError(ConfigurationError::InvalidData, "Cannot save style config: currentStyle is null");
        Logger::LogError("Cannot save style config: currentStyle is null");
        return false;
    }
    
    try {
        // Create backup before saving
        if (!CreateBackupConfiguration()) {
            Logger::LogWarning("Failed to create backup configuration, proceeding with save anyway");
        }
        
        // Update current style with ImGui's current state before saving
        currentStyle->LoadFromImGui();
        
        // Validate and sanitize the configuration before saving
        if (!ValidateAndSanitizeInput(*currentStyle)) {
            SetError(ConfigurationError::ValidationFailed, "Configuration validation failed", "Style properties contain invalid values");
            return false;
        }
        
        auto fullFilePath = Engine::GetEnginePath() / ENGINE_CONFIG_FILE_NAME;
        
        // Check if we have write permissions
        if (std::filesystem::exists(fullFilePath.string() + ".exfile")) {
            auto perms = std::filesystem::status(fullFilePath.string() + ".exfile").permissions();
            if ((perms & std::filesystem::perms::owner_write) == std::filesystem::perms::none) {
                SetError(ConfigurationError::PermissionDenied, "No write permission for configuration file", fullFilePath.string() + ".exfile");
                Logger::LogError("No write permission for configuration file: " + fullFilePath.string() + ".exfile");
                return false;
            }
        }
        
        auto jsonData = JsonUtility::ToJson(*currentStyle);
        bool saveResult = FileManagement::SaveFile(fullFilePath, jsonData.dump());
        
        if (saveResult) {
            Logger::Log("Style configuration saved successfully to: " + fullFilePath.string() + ".exfile");
            return true;
        } else {
            SetError(ConfigurationError::UnknownError, "Failed to save style configuration", fullFilePath.string() + ".exfile");
            Logger::LogError("Failed to save style configuration to: " + fullFilePath.string() + ".exfile");
            return false;
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        SetError(ConfigurationError::PermissionDenied, "Filesystem error during save", e.what());
        Logger::LogError("Filesystem error during save: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        SetError(ConfigurationError::UnknownError, "Exception during save", e.what());
        Logger::LogError("Failed to save style configuration: " + std::string(e.what()));
        return false;
    }
}

bool ConfigurationManager::LoadStyleConfig() {
    ClearError();
    
    if (currentStyle == nullptr) {
        currentStyle = std::make_unique<StyleConfiguration>();
    }
    
    try {
        auto fullFilePath = Engine::GetEnginePath() / ENGINE_CONFIG_FILE_NAME;
        std::string fullFilePathStr = fullFilePath.string() + ".exfile";
        
        Logger::Log("Attempting to load style configuration from: " + fullFilePathStr);
        
        // Check if file exists
        if (!std::filesystem::exists(fullFilePathStr)) {
            SetError(ConfigurationError::FileNotFound, "Configuration file not found", fullFilePathStr);
            Logger::Log("Configuration file not found, using defaults: " + fullFilePathStr);
            currentStyle->LoadFromImGui();
            return false;
        }
        
        // Check if file is corrupted before attempting to load
        if (IsConfigurationCorrupted(fullFilePathStr)) {
            SetError(ConfigurationError::FileCorrupted, "Configuration file appears to be corrupted", fullFilePathStr);
            Logger::LogError("Configuration file is corrupted, attempting to restore from backup");
            
            // Try to restore from backup
            if (RestoreFromBackup()) {
                Logger::Log("Successfully restored configuration from backup");
                return LoadStyleConfig(); // Recursive call to load the restored backup
            } else {
                Logger::LogError("Failed to restore from backup, using defaults");
                currentStyle->LoadFromImGui();
                return false;
            }
        }
        
        // Check read permissions
        auto perms = std::filesystem::status(fullFilePathStr).permissions();
        if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
            SetError(ConfigurationError::PermissionDenied, "No read permission for configuration file", fullFilePathStr);
            Logger::LogError("No read permission for configuration file: " + fullFilePathStr);
            currentStyle->LoadFromImGui();
            return false;
        }
        
        // Use FileManagement to ensure consistent file handling with .exfile extension
        bool loadResult = FileManagement::LoadFromJson(fullFilePath, *currentStyle);
        
        if (loadResult) {
            // Validate and sanitize loaded configuration before applying
            if (ValidateAndSanitizeInput(*currentStyle)) {
                // Apply the loaded configuration to ImGui
                currentStyle->ApplyToImGui();
                Logger::Log("Style configuration loaded and applied successfully from: " + fullFilePathStr);
                return true;
            } else {
                SetError(ConfigurationError::ValidationFailed, "Loaded configuration contains invalid data", "Configuration was sanitized and defaults applied");
                Logger::LogError("Loaded style configuration contains invalid data, sanitized and using corrected values");
                // Configuration was sanitized by ValidateAndSanitizeInput, so apply it
                currentStyle->ApplyToImGui();
                return true; // Still return true since we recovered
            }
        } else {
            SetError(ConfigurationError::InvalidData, "Failed to parse configuration file", fullFilePathStr);
            Logger::LogError("Failed to parse configuration file, using defaults");
            currentStyle->LoadFromImGui();
            return false;
        }
        
    } catch (const std::filesystem::filesystem_error& e) {
        SetError(ConfigurationError::PermissionDenied, "Filesystem error during load", e.what());
        Logger::LogError("Filesystem error while loading configuration: " + std::string(e.what()) + ", using defaults");
        currentStyle->LoadFromImGui();
        return false;
    } catch (const std::exception& e) {
        SetError(ConfigurationError::UnknownError, "Exception during load", e.what());
        Logger::LogError("Exception while loading style configuration: " + std::string(e.what()) + ", using defaults");
        currentStyle->LoadFromImGui();
        return false;
    }
}

void ConfigurationManager::ResetStyleToDefaults() {
    if (!initialized) {
        Initialize();
    }
    
    if (currentStyle == nullptr) {
        currentStyle = std::make_unique<StyleConfiguration>();
    }
    
    currentStyle->ResetToDefaults();
    
    ApplyStyleChanges();
    
    Logger::Log("Style configuration reset to defaults");
}

bool ConfigurationManager::ApplyStyleChanges() {
    ClearError();
    
    if (!initialized) {
        Initialize();
    }
    
    if (currentStyle == nullptr) {
        SetError(ConfigurationError::InvalidData, "Cannot apply style changes: currentStyle is null");
        Logger::LogError("Cannot apply style changes: currentStyle is null");
        return false;
    }
    
    // Validate and sanitize configuration before applying
    if (!ValidateAndSanitizeInput(*currentStyle)) {
        SetError(ConfigurationError::ValidationFailed, "Cannot apply style changes: configuration validation failed");
        Logger::LogError("Cannot apply style changes: configuration is invalid");
        return false;
    }
    
    Logger::Log("Applying style changes to ImGui...");
    
    try {
        currentStyle->ApplyToImGui();
        
        if (!SaveStyleConfig()) {
            Logger::LogWarning("Style changes applied but failed to save to file");
        }
        
        ForceRefreshAllWindows();
        
        return true;
        
    } catch (const std::exception& e) {
        SetError(ConfigurationError::UnknownError, "Exception while applying style changes", e.what());
        Logger::LogError("Exception while applying style changes: " + std::string(e.what()));
        return false;
    }
}

void ConfigurationManager::ForceRefreshAllWindows() {
    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        for (int i = 0; i < platform_io.Viewports.Size; i++) {
            ImGuiViewport* viewport = platform_io.Viewports[i];
            if (viewport->PlatformUserData) {
                viewport->Flags |= ImGuiViewportFlags_NoAutoMerge;
            }
        }
    }
}

bool ConfigurationManager::ValidateStyleConfiguration(const StyleConfiguration& config) {
    for (const auto& colorPair : config.colors) {
        if (!ValidateColorValue(colorPair.second)) {
            Logger::Log("Invalid color value found in configuration");
            return false;
        }
    }
    
    if (!ValidateSizeValue(config.windowRounding) || 
        !ValidateSizeValue(config.windowBorderSize) ||
        !ValidateSizeValue(config.frameRounding) ||
        !ValidateSizeValue(config.frameBorderSize) ||
        !ValidateSizeValue(config.indentSpacing) ||
        !ValidateSizeValue(config.scrollbarSize) ||
        !ValidateSizeValue(config.scrollbarRounding) ||
        !ValidateSizeValue(config.grabMinSize) ||
        !ValidateSizeValue(config.grabRounding) ||
        !ValidateSizeValue(config.tabRounding) ||
        !ValidateSizeValue(config.tabBorderSize)) {
        Logger::Log("Invalid size value found in configuration");
        return false;
    }
    
    if (config.alpha < 0.0f || config.alpha > 1.0f ||
        config.disabledAlpha < 0.0f || config.disabledAlpha > 1.0f) {
        Logger::Log("Invalid alpha value found in configuration");
        return false;
    }
    
    if (config.buttonTextAlign.x < 0.0f || config.buttonTextAlign.x > 1.0f ||
        config.buttonTextAlign.y < 0.0f || config.buttonTextAlign.y > 1.0f ||
        config.selectableTextAlign.x < 0.0f || config.selectableTextAlign.x > 1.0f ||
        config.selectableTextAlign.y < 0.0f || config.selectableTextAlign.y > 1.0f) {
        Logger::Log("Invalid alignment value found in configuration");
        return false;
    }
    
    return true;
}

bool ConfigurationManager::ValidateColorValue(const ImVec4& color) {
    return (color.x >= 0.0f && color.x <= 1.0f &&
            color.y >= 0.0f && color.y <= 1.0f &&
            color.z >= 0.0f && color.z <= 1.0f &&
            color.w >= 0.0f && color.w <= 1.0f);
}

bool ConfigurationManager::ValidateSizeValue(float value) {
    return (value >= 0.0f && value <= 1000.0f);
}

void ConfigurationManager::ForceSaveCurrentStyle() {
    if (!initialized) {
        Initialize();
    }
    
    Logger::Log("Force saving current ImGui style configuration...");
    SaveStyleConfig();
}

// Error handling methods
void ConfigurationManager::SetError(ConfigurationError::Type type, const std::string& message, const std::string& details) {
    lastError.type = type;
    lastError.message = message;
    lastError.details = details;
}

void ConfigurationManager::ClearError() {
    lastError.Clear();
}

const ConfigurationError& ConfigurationManager::GetLastError() {
    return lastError;
}

bool ConfigurationManager::HasError() {
    return lastError.HasError();
}

std::string ConfigurationManager::GetErrorMessage() {
    if (!lastError.HasError()) {
        return "";
    }
    
    std::string fullMessage = lastError.message;
    if (!lastError.details.empty()) {
        fullMessage += " (" + lastError.details + ")";
    }
    return fullMessage;
}

void ConfigurationManager::ClearLastError() {
    ClearError();
}

// Backup and restore methods
bool ConfigurationManager::CreateBackupConfiguration() {
    try {
        auto enginePath = Engine::GetEnginePath();
        auto originalPath = enginePath / (ENGINE_CONFIG_FILE_NAME + std::string(".exfile"));
        auto backupPath = enginePath / (ENGINE_CONFIG_FILE_NAME + std::string("_backup.exfile"));
        
        if (std::filesystem::exists(originalPath)) {
            std::filesystem::copy_file(originalPath, backupPath, std::filesystem::copy_options::overwrite_existing);
            Logger::Log("Created backup configuration: " + backupPath.string());
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        Logger::LogError("Failed to create backup configuration: " + std::string(e.what()));
        return false;
    }
}

bool ConfigurationManager::RestoreFromBackup() {
    try {
        auto enginePath = Engine::GetEnginePath();
        auto originalPath = enginePath / (ENGINE_CONFIG_FILE_NAME + std::string(".exfile"));
        auto backupPath = enginePath / (ENGINE_CONFIG_FILE_NAME + std::string("_backup.exfile"));
        
        if (std::filesystem::exists(backupPath)) {
            std::filesystem::copy_file(backupPath, originalPath, std::filesystem::copy_options::overwrite_existing);
            Logger::Log("Restored configuration from backup: " + backupPath.string());
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        Logger::LogError("Failed to restore from backup configuration: " + std::string(e.what()));
        return false;
    }
}

bool ConfigurationManager::IsConfigurationCorrupted(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return true;
        }
        
        nlohmann::json json;
        file >> json;
        
        if (!json.is_object()) {
            return true;
        }

        if (!json.contains("colors") && !json.contains("windowPadding") && !json.contains("alpha")) {
            return true;
        }
        
        return false;
    } catch (const std::exception&) {
        return true; 
    }
}

bool ConfigurationManager::ValidateAndSanitizeInput(StyleConfiguration& config) {
    bool wasModified = false;
    
    for (auto& colorPair : config.colors) {
        ImVec4 originalColor = colorPair.second;
        ImVec4 sanitizedColor = SanitizeColorValue(originalColor);
        if (originalColor.x != sanitizedColor.x || originalColor.y != sanitizedColor.y || 
            originalColor.z != sanitizedColor.z || originalColor.w != sanitizedColor.w) {
            colorPair.second = sanitizedColor;
            wasModified = true;
        }
    }
    
    config.windowRounding = SanitizeSizeValue(config.windowRounding);
    config.windowBorderSize = SanitizeSizeValue(config.windowBorderSize);
    config.frameRounding = SanitizeSizeValue(config.frameRounding);
    config.frameBorderSize = SanitizeSizeValue(config.frameBorderSize);
    config.indentSpacing = SanitizeSizeValue(config.indentSpacing);
    config.scrollbarSize = SanitizeSizeValue(config.scrollbarSize);
    config.scrollbarRounding = SanitizeSizeValue(config.scrollbarRounding);
    config.grabMinSize = SanitizeSizeValue(config.grabMinSize);
    config.grabRounding = SanitizeSizeValue(config.grabRounding);
    config.tabRounding = SanitizeSizeValue(config.tabRounding);
    config.tabBorderSize = SanitizeSizeValue(config.tabBorderSize);
    
    config.alpha = SanitizeAlphaValue(config.alpha);
    config.disabledAlpha = SanitizeAlphaValue(config.disabledAlpha);
    
    config.buttonTextAlign = SanitizeAlignmentValue(config.buttonTextAlign);
    config.selectableTextAlign = SanitizeAlignmentValue(config.selectableTextAlign);

    config.windowPadding.x = SanitizeSizeValue(config.windowPadding.x);
    config.windowPadding.y = SanitizeSizeValue(config.windowPadding.y);
    config.framePadding.x = SanitizeSizeValue(config.framePadding.x);
    config.framePadding.y = SanitizeSizeValue(config.framePadding.y);
    config.itemSpacing.x = SanitizeSizeValue(config.itemSpacing.x);
    config.itemSpacing.y = SanitizeSizeValue(config.itemSpacing.y);
    config.itemInnerSpacing.x = SanitizeSizeValue(config.itemInnerSpacing.x);
    config.itemInnerSpacing.y = SanitizeSizeValue(config.itemInnerSpacing.y);
    
    return true; // Always return true after sanitization
}

bool ConfigurationManager::ValidateAlignmentValue(const ImVec2& alignment) {
    return (alignment.x >= 0.0f && alignment.x <= 1.0f &&
            alignment.y >= 0.0f && alignment.y <= 1.0f);
}

bool ConfigurationManager::ValidateAlphaValue(float alpha) {
    return (alpha >= 0.0f && alpha <= 1.0f);
}

// Input sanitization methods
ImVec4 ConfigurationManager::SanitizeColorValue(const ImVec4& color) {
    return ImVec4(
        std::clamp(color.x, 0.0f, 1.0f),
        std::clamp(color.y, 0.0f, 1.0f),
        std::clamp(color.z, 0.0f, 1.0f),
        std::clamp(color.w, 0.0f, 1.0f)
    );
}

float ConfigurationManager::SanitizeSizeValue(float value) {
    return std::clamp(value, 0.0f, 1000.0f);
}

ImVec2 ConfigurationManager::SanitizeAlignmentValue(const ImVec2& alignment) {
    return ImVec2(
        std::clamp(alignment.x, 0.0f, 1.0f),
        std::clamp(alignment.y, 0.0f, 1.0f)
    );
}

float ConfigurationManager::SanitizeAlphaValue(float alpha) {
    return std::clamp(alpha, 0.0f, 1.0f);
}