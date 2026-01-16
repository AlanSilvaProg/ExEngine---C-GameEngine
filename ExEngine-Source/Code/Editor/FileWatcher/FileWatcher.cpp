#include "FileWatcher.h"
#include "FileWatcherError.h"
#include "FileWatcherConfig.h"
#include "PlatformWatcherFactory.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>
#include <algorithm>
#include <regex>

FileWatcher::FileWatcher() 
    : m_running(false) {
}

FileWatcher::~FileWatcher() {
    Stop();
}

bool FileWatcher::AddWatchPath(const std::string& path, bool recursive) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    // Validate path exists
    if (!std::filesystem::exists(path)) {
        Logger::LogError("FileWatcher: Path does not exist: " + path);
        return false;
    }
    
    // Check if path is already being watched
    auto it = std::find(m_config.watchPaths.begin(), m_config.watchPaths.end(), path);
    if (it != m_config.watchPaths.end()) {
        return true; // Already watching
    }
    
    m_config.watchPaths.push_back(path);
    m_config.recursive = recursive;
    
    // If watcher is running, add path to platform watcher
    if (m_running && m_platformWatcher) {
        return m_platformWatcher->AddPath(path);
    }
    
    return true;
}

bool FileWatcher::RemoveWatchPath(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    auto it = std::find(m_config.watchPaths.begin(), m_config.watchPaths.end(), path);
    if (it == m_config.watchPaths.end()) {
        Logger::LogWarning("FileWatcher: Attempted to remove non-watched path: " + path);
        return false; // Path not found
    }
    
    m_config.watchPaths.erase(it);
    
    // If watcher is running, remove path from platform watcher
    if (m_running && m_platformWatcher) {
        return m_platformWatcher->RemovePath(path);
    }
    
    return true;
}

void FileWatcher::SetFileFilter(const std::vector<std::string>& extensions) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    m_config.fileExtensions = extensions;
}

void FileWatcher::SetExcludePatterns(const std::vector<std::string>& patterns) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    m_config.excludePatterns = patterns;
}

void FileWatcher::EnableDebugLogging(bool enable) {
    // Debug logging is now handled by the engine's Logger
    // This method is kept for API compatibility but doesn't do anything
}

bool FileWatcher::SaveConfiguration(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    std::string configPath = filePath.empty() ? 
        FileWatcherConfig::GetDefaultConfigPath() : filePath;
    
    return FileWatcherConfig::SaveConfig(m_config, configPath);
}

bool FileWatcher::LoadConfiguration(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    std::string configPath = filePath.empty() ? 
        FileWatcherConfig::GetDefaultConfigPath() : filePath;
    
    return FileWatcherConfig::LoadConfig(m_config, configPath);
}

bool FileWatcher::Start() {
    if (m_running) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    if (m_config.watchPaths.empty()) {
        Logger::LogError("FileWatcher: No paths to watch");
        return false;
    }
    
    // Create platform-specific watcher
    m_platformWatcher = PlatformWatcherFactory::Create();
    if (!m_platformWatcher) {
        Logger::LogError("FileWatcher: Failed to create platform watcher");
        return false;
    }
    
    // Initialize platform watcher
    if (!m_platformWatcher->Initialize(m_config.watchPaths)) {
        Logger::LogError("FileWatcher: Failed to initialize platform watcher");
        return false;
    }
    
    // Start worker thread
    m_running = true;
    m_workerThread = std::thread(&FileWatcher::WorkerThreadFunction, this);
    
    std::string pathsStr = "FileWatcher: Started monitoring " + std::to_string(m_config.watchPaths.size()) + " paths";
    for (const auto& path : m_config.watchPaths) {
        pathsStr += "\n  - " + path;
    }
    return true;
}

void FileWatcher::Stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    if (m_platformWatcher) {
        m_platformWatcher->Stop();
    }
    
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    m_platformWatcher.reset();
}

bool FileWatcher::IsRunning() const {
    return m_running;
}

void FileWatcher::WorkerThreadFunction() {
    if (!m_platformWatcher) {
        return;
    }
    
    m_platformWatcher->Run([this](const FileEvent& event) {
        ProcessEvent(event);
    });
}

void FileWatcher::ProcessEvent(const FileEvent& event) {
    // Check if file should be filtered
    if (ShouldFilterFile(event.filePath)) {
        return;
    }
    
    // Handle directory deletion with proper event ordering
    if (event.type == FileEventType::Deleted && event.IsDirectory()) {
        // First emit events for all files in the directory
        try {
            if (std::filesystem::exists(event.filePath)) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(event.filePath)) {
                    if (entry.is_regular_file()) {
                        FileEvent fileDeleteEvent;
                        fileDeleteEvent.type = FileEventType::Deleted;
                        fileDeleteEvent.filePath = entry.path().string();
                        fileDeleteEvent.timestamp = event.timestamp;
                        fileDeleteEvent.fileSize = 0;
                        
                        DispatchEvent(fileDeleteEvent);
                    }
                }
            }
        } catch (const std::exception& e) {
            Logger::LogError("FileWatcher: Error processing directory deletion: " + std::string(e.what()));
        }
    }
    
    // Handle dynamic directory monitoring
    if (event.type == FileEventType::Created && event.IsDirectory()) {
        // Automatically start monitoring new directories
        if (m_platformWatcher) {
            m_platformWatcher->AddPath(event.filePath);
        }
    } else if (event.type == FileEventType::Deleted && event.IsDirectory()) {
        // Stop monitoring deleted directories
        if (m_platformWatcher) {
            m_platformWatcher->RemovePath(event.filePath);
        }
        
        // Clean up debounce entries for files in this directory
        {
            std::lock_guard<std::mutex> lock(m_debounceMutex);
            auto it = m_lastEventTime.begin();
            while (it != m_lastEventTime.end()) {
                if (it->first.find(event.filePath) == 0) {
                    it = m_lastEventTime.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    // Handle rename operations with proper old/new path tracking
    if (event.type == FileEventType::Renamed && !event.oldPath.empty()) {
        // Create separate delete and create events for rename
        FileEvent deleteEvent = event;
        deleteEvent.type = FileEventType::Deleted;
        deleteEvent.filePath = event.oldPath;
        
        FileEvent createEvent = event;
        createEvent.type = FileEventType::Created;
        // filePath already contains the new path
        
        // Dispatch delete first, then create
        DispatchEvent(deleteEvent);
        DispatchEvent(createEvent);
        return; // Don't process the original rename event
    }
    
    // Check debouncing
    {
        std::lock_guard<std::mutex> lock(m_debounceMutex);
        auto now = std::chrono::system_clock::now();
        auto it = m_lastEventTime.find(event.filePath);
        
        if (it != m_lastEventTime.end()) {
            auto timeSinceLastEvent = now - it->second;
            if (timeSinceLastEvent < m_config.debounceTime) {
                return; // Skip this event due to debouncing
            }
        }
        
        m_lastEventTime[event.filePath] = now;
    }
    
    // Queue event for main thread processing
    {
        std::lock_guard<std::mutex> lock(m_eventQueueMutex);
        m_eventQueue.push(event);
    }
    
    // Dispatch event immediately (events are thread-safe)
    DispatchEvent(event);
}

bool FileWatcher::ShouldFilterFile(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(m_configMutex);
    
    // Check file extensions filter
    if (!m_config.fileExtensions.empty()) {
        std::filesystem::path path(filePath);
        std::string extension = path.extension().string();
        
        auto it = std::find(m_config.fileExtensions.begin(), m_config.fileExtensions.end(), extension);
        if (it == m_config.fileExtensions.end()) {
            return true; // Filter out this file
        }
    }
    
    // Check exclude patterns
    for (const auto& pattern : m_config.excludePatterns) {
        try {
            std::regex regex(pattern);
            if (std::regex_search(filePath, regex)) {
                return true; // Filter out this file
            }
        } catch (const std::regex_error& e) {
            Logger::LogError("FileWatcher: Invalid regex pattern: " + pattern);
        }
    }
    
    // Check hidden files
    if (!m_config.watchHiddenFiles) {
        std::filesystem::path path(filePath);
        std::string filename = path.filename().string();
        if (!filename.empty() && filename[0] == '.') {
            return true; // Filter out hidden files
        }
    }
    
    return false;
}

void FileWatcher::DispatchEvent(const FileEvent& event) {
    try {
        // Log the event
        std::string eventTypeStr;
        switch (event.type) {
            case FileEventType::Created: eventTypeStr = "CREATED"; break;
            case FileEventType::Modified: eventTypeStr = "MODIFIED"; break;
            case FileEventType::Deleted: eventTypeStr = "DELETED"; break;
            case FileEventType::Renamed: eventTypeStr = "RENAMED"; break;
        }
        
        switch (event.type) {
            case FileEventType::Created:
                OnFileCreated.Invoke(event);
                break;
            case FileEventType::Modified:
                OnFileChanged.Invoke(event);
                break;
            case FileEventType::Deleted:
                OnFileDeleted.Invoke(event);
                break;
            case FileEventType::Renamed:
                // Treat rename as delete + create
                OnFileDeleted.Invoke(event);
                OnFileCreated.Invoke(event);
                break;
        }
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Error dispatching event: " + std::string(e.what()));
    }
}