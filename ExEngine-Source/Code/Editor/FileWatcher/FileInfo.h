#pragma once
#include <filesystem>

struct FileWatcherInfo {
    std::filesystem::file_time_type lastWriteTime;
    uintmax_t fileSize;
    bool isDirectory;
    
    bool operator!=(const FileWatcherInfo& other) const {
        return lastWriteTime != other.lastWriteTime || 
               fileSize != other.fileSize || 
               isDirectory != other.isDirectory;
    }
    
    bool operator==(const FileWatcherInfo& other) const {
        return !(*this != other);
    }
};