#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <map>
#include "../../Engine/Core/EventSystem/Event.h"
#include "FileEvent.h"
#include "WatchConfig.h"
#include "IPlatformWatcher.h"

class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();
    
    // Configuration
    bool AddWatchPath(const std::string& path, bool recursive = true);
    bool RemoveWatchPath(const std::string& path);
    void SetFileFilter(const std::vector<std::string>& extensions);
    void SetExcludePatterns(const std::vector<std::string>& patterns);
    void EnableDebugLogging(bool enable);
    
    // Configuration persistence
    bool SaveConfiguration(const std::string& filePath = "");
    bool LoadConfiguration(const std::string& filePath = "");
    
    // Lifecycle
    bool Start();
    void Stop();
    bool IsRunning() const;
    
    // Events
    Event<const FileEvent&> OnFileChanged;
    Event<const FileEvent&> OnFileCreated;
    Event<const FileEvent&> OnFileDeleted;
    
private:
    void WorkerThreadFunction();
    void ProcessEvent(const FileEvent& event);
    bool ShouldFilterFile(const std::string& filePath) const;
    void DispatchEvent(const FileEvent& event);
    
    std::unique_ptr<IPlatformWatcher> m_platformWatcher;
    std::thread m_workerThread;
    std::atomic<bool> m_running;
    mutable std::mutex m_configMutex;
    mutable std::mutex m_eventQueueMutex;
    
    WatchConfig m_config;
    std::queue<FileEvent> m_eventQueue;
    
    // Debouncing support
    std::mutex m_debounceMutex;
    std::map<std::string, std::chrono::system_clock::time_point> m_lastEventTime;
};