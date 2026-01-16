#include "PollingFileWatcher.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>
#include <algorithm>

PollingFileWatcher::PollingFileWatcher(std::chrono::milliseconds pollInterval)
    : m_pollInterval(pollInterval), m_running(false) {
}

PollingFileWatcher::~PollingFileWatcher() {
    Stop();
}

bool PollingFileWatcher::Initialize(const std::vector<std::string>& paths) {
    m_watchPaths = paths;
    
    // Validate all paths exist
    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            Logger::LogError("PollingFileWatcher: Path does not exist: " + path);
            return false;
        }
    }
    
    // Build initial file cache
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    for (const auto& path : m_watchPaths) {
        ScanDirectory(path);
    }
    
    return true;
}

void PollingFileWatcher::Run(std::function<void(FileEvent)> eventCallback) {
    m_eventCallback = eventCallback;
    m_running = true;
    
    m_pollThread = std::thread(&PollingFileWatcher::PollLoop, this);
}

void PollingFileWatcher::Stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    m_fileCache.clear();
}

bool PollingFileWatcher::AddPath(const std::string& path) {
    auto it = std::find(m_watchPaths.begin(), m_watchPaths.end(), path);
    if (it != m_watchPaths.end()) {
        return true; // Already watching
    }
    
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    m_watchPaths.push_back(path);
    
    // Scan the new path and add to cache
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    ScanDirectory(path);
    
    return true;
}

bool PollingFileWatcher::RemovePath(const std::string& path) {
    auto it = std::find(m_watchPaths.begin(), m_watchPaths.end(), path);
    if (it == m_watchPaths.end()) {
        return false;
    }
    
    m_watchPaths.erase(it);
    
    // Remove files under this path from cache
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    auto cacheIt = m_fileCache.begin();
    while (cacheIt != m_fileCache.end()) {
        if (cacheIt->first.find(path) == 0) {
            cacheIt = m_fileCache.erase(cacheIt);
        } else {
            ++cacheIt;
        }
    }
    
    return true;
}

void PollingFileWatcher::SetPollInterval(std::chrono::milliseconds interval) {
    m_pollInterval = interval;
}

void PollingFileWatcher::PollLoop() {
    while (m_running) {
        auto startTime = std::chrono::steady_clock::now();
        
        CheckFileChanges();
        
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        // Sleep for the remaining time
        auto sleepTime = m_pollInterval - elapsed;
        if (sleepTime > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
}

void PollingFileWatcher::ScanDirectory(const std::string& path) {
    try {
        if (std::filesystem::is_regular_file(path)) {
            // Single file
            FileWatcherInfo info = GetFileInfo(path);
            m_fileCache[path] = info;
        } else if (std::filesystem::is_directory(path)) {
            // Directory - scan recursively
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_regular_file() || entry.is_directory()) {
                    std::string filePath = entry.path().string();
                    FileWatcherInfo info = GetFileInfo(filePath);
                    m_fileCache[filePath] = info;
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::LogError("PollingFileWatcher: Error scanning directory " + path + 
                  ": " + std::string(e.what()));
    }
}

void PollingFileWatcher::CheckFileChanges() {
    std::lock_guard<std::mutex> lock(m_cacheMutex);
    
    // Build current file state
    std::unordered_map<std::string, FileWatcherInfo> currentFiles;
    
    for (const auto& path : m_watchPaths) {
        try {
            if (std::filesystem::exists(path)) {
                if (std::filesystem::is_regular_file(path)) {
                    FileWatcherInfo info = GetFileInfo(path);
                    currentFiles[path] = info;
                } else if (std::filesystem::is_directory(path)) {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                        if (entry.is_regular_file() || entry.is_directory()) {
                            std::string filePath = entry.path().string();
                            FileWatcherInfo info = GetFileInfo(filePath);
                            currentFiles[filePath] = info;
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            Logger::LogError("PollingFileWatcher: Error checking path " + path + 
                      ": " + std::string(e.what()));
        }
    }
    
    // Check for new and modified files
    for (const auto& pair : currentFiles) {
        const std::string& filePath = pair.first;
        const FileWatcherInfo& currentInfo = pair.second;
        
        auto it = m_fileCache.find(filePath);
        if (it == m_fileCache.end()) {
            // New file
            ProcessNewFile(filePath, currentInfo);
        } else if (it->second != currentInfo) {
            // Modified file
            ProcessFileChange(filePath, it->second, currentInfo);
        }
    }
    
    // Check for deleted files
    for (const auto& pair : m_fileCache) {
        const std::string& filePath = pair.first;
        
        if (currentFiles.find(filePath) == currentFiles.end()) {
            // File was deleted
            ProcessDeletedFile(filePath);
        }
    }
    
    // Update cache
    m_fileCache = std::move(currentFiles);
}

FileWatcherInfo PollingFileWatcher::GetFileInfo(const std::string& path) {
    FileWatcherInfo info;
    
    try {
        if (std::filesystem::exists(path)) {
            info.lastWriteTime = std::filesystem::last_write_time(path);
            info.isDirectory = std::filesystem::is_directory(path);
            
            if (!info.isDirectory) {
                info.fileSize = std::filesystem::file_size(path);
            } else {
                info.fileSize = 0;
            }
        }
    } catch (const std::exception&) {
        // Use default values if we can't get file info
        info.fileSize = 0;
        info.isDirectory = false;
    }
    
    return info;
}

void PollingFileWatcher::ProcessFileChange(const std::string& path, 
                                          const FileWatcherInfo& oldInfo, 
                                          const FileWatcherInfo& newInfo) {
    if (!m_eventCallback) {
        return;
    }
    
    FileEvent event;
    event.filePath = path;
    event.timestamp = std::chrono::system_clock::now();
    event.type = FileEventType::Modified;
    event.fileSize = newInfo.fileSize;
    
    m_eventCallback(event);
}

void PollingFileWatcher::ProcessNewFile(const std::string& path, const FileWatcherInfo& info) {
    if (!m_eventCallback) {
        return;
    }
    
    FileEvent event;
    event.filePath = path;
    event.timestamp = std::chrono::system_clock::now();
    event.type = FileEventType::Created;
    event.fileSize = info.fileSize;
    
    m_eventCallback(event);
}

void PollingFileWatcher::ProcessDeletedFile(const std::string& path) {
    if (!m_eventCallback) {
        return;
    }
    
    FileEvent event;
    event.filePath = path;
    event.timestamp = std::chrono::system_clock::now();
    event.type = FileEventType::Deleted;
    event.fileSize = 0;
    
    m_eventCallback(event);
}