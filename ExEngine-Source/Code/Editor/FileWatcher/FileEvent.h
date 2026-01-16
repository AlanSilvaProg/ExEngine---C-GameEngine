#pragma once
#include <string>
#include <chrono>
#include <filesystem>

enum class FileEventType {
    Created,
    Modified,
    Deleted,
    Renamed
};

struct FileEvent {
    FileEventType type;
    std::string filePath;
    std::string oldPath;  // For rename events
    std::chrono::system_clock::time_point timestamp;
    size_t fileSize;      // For created/modified events
    
    // Utility methods
    bool IsDirectory() const {
        return std::filesystem::is_directory(filePath);
    }
    
    std::string GetExtension() const {
        std::filesystem::path path(filePath);
        return path.extension().string();
    }
    
    std::string GetFileName() const {
        std::filesystem::path path(filePath);
        return path.filename().string();
    }
};