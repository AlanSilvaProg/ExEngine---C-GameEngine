#pragma once
#include "StyleConfiguration.h"
#include <memory>
#include <string>

#ifndef ENGINE_CONFIG_FILE_NAME
#define ENGINE_CONFIG_FILE_NAME "EngineConfig"
#endif

struct ConfigurationError {
    enum Type {
        None,
        FileNotFound,
        FileCorrupted,
        InvalidData,
        PermissionDenied,
        ValidationFailed,
        UnknownError
    };
    
    Type type = None;
    std::string message;
    std::string details;
    
    ConfigurationError() = default;
    ConfigurationError(Type t, const std::string& msg, const std::string& det = "") 
        : type(t), message(msg), details(det) {}
    
    bool HasError() const { return type != None; }
    void Clear() { type = None; message.clear(); details.clear(); }
};

class ConfigurationManager {
private:
    static std::unique_ptr<StyleConfiguration> currentStyle;
    static std::string configFilePath;
    static bool initialized;
    static ConfigurationError lastError;
    
    // Validation helper methods
    static bool ValidateStyleConfiguration(const StyleConfiguration& config);
    static bool ValidateColorValue(const ImVec4& color);
    static bool ValidateSizeValue(float value);
    static bool ValidateAlignmentValue(const ImVec2& alignment);
    static bool ValidateAlphaValue(float alpha);
    
    // Error handling methods
    static void SetError(ConfigurationError::Type type, const std::string& message, const std::string& details = "");
    static void ClearError();
    static bool CreateBackupConfiguration();
    static bool RestoreFromBackup();
    
    // Input sanitization methods
    static ImVec4 SanitizeColorValue(const ImVec4& color);
    static float SanitizeSizeValue(float value);
    static ImVec2 SanitizeAlignmentValue(const ImVec2& alignment);
    static float SanitizeAlphaValue(float alpha);
    
public:
    static void Initialize();
    static StyleConfiguration& GetStyleConfig();
    static bool SaveStyleConfig();
    static bool LoadStyleConfig();
    static void ResetStyleToDefaults();
    static bool ApplyStyleChanges();
    static void ForceRefreshAllWindows();
    static void ForceSaveCurrentStyle();
    
    static const ConfigurationError& GetLastError();
    static bool HasError();
    static std::string GetErrorMessage();
    static void ClearLastError();
    
    static bool ValidateAndSanitizeInput(StyleConfiguration& config);
    static bool IsConfigurationCorrupted(const std::string& filePath);
};