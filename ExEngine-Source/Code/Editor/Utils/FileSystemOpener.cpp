#include "FileSystemOpener.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>

// Platform-specific includes for file opening
#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <cstdlib>
#endif

bool FileSystemOpener::OpenFileInSystemEditor(const std::filesystem::path& filePath) {
    // Convert to absolute path if relative - ensures consistent behavior across platforms
    std::filesystem::path absolutePath;
    try {
        if (filePath.is_relative()) {
            absolutePath = std::filesystem::absolute(filePath);
        } else {
            absolutePath = filePath;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::Log("Failed to resolve file path: " + std::string(e.what()));
        return false;
    }
    
    // Validate file existence before attempting to open
    if (!std::filesystem::exists(absolutePath)) {
        Logger::Log("File does not exist: " + absolutePath.string());
        return false;
    }
    
    // Platform-specific file opening implementation
    bool success = false;
    std::string errorMessage;
    
#ifdef _WIN32
    // Windows implementation: ShellExecuteW provides Unicode support and proper file association handling
    std::wstring wideFilePath = absolutePath.wstring();
    HINSTANCE result = ShellExecuteW(NULL, L"open", wideFilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    // ShellExecuteW returns a value > 32 on success, <= 32 indicates various error conditions
    if (reinterpret_cast<uintptr_t>(result) > 32) {
        success = true;
        Logger::Log("Successfully opened file in system editor: " + absolutePath.string());
    } else {
        errorMessage = "Windows ShellExecuteW failed with error code: " + std::to_string(reinterpret_cast<uintptr_t>(result));
    }
    
#elif defined(__APPLE__)
    // macOS implementation: 'open' command is built-in and handles file associations automatically
    // Quotes around path handle spaces and special characters in filenames
    std::string command = "open \"" + absolutePath.string() + "\"";
    int result = system(command.c_str());
    
    if (result == 0) {
        success = true;
        Logger::Log("Successfully opened file in system editor: " + absolutePath.string());
    } else {
        errorMessage = "macOS 'open' command failed with exit code: " + std::to_string(result);
    }
    
#elif defined(__linux__)
    // Linux implementation: 'xdg-open' is the standard way to open files with default applications
    // Works across different desktop environments (GNOME, KDE, XFCE, etc.)
    std::string command = "xdg-open \"" + absolutePath.string() + "\"";
    int result = system(command.c_str());
    
    if (result == 0) {
        success = true;
        Logger::Log("Successfully opened file in system editor: " + absolutePath.string());
    } else {
        errorMessage = "Linux 'xdg-open' command failed with exit code: " + std::to_string(result);
    }
    
#else
    // Fallback for unsupported platforms - provides clear error message for debugging
    errorMessage = "Unsupported platform for opening files in system editor";
#endif

    // Log error if opening failed
    if (!success) {
        Logger::Log("Failed to open file in system editor: " + absolutePath.string() + " - " + errorMessage);
    }
    
    return success;
}