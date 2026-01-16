#pragma once
#include "IPlatformWatcher.h"
#include "FileInfo.h"
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <thread>

class PollingFileWatcher : public IPlatformWatcher {
public:
    PollingFileWatcher(std::chrono::milliseconds pollInterval = std::chrono::milliseconds(1000));
    ~PollingFileWatcher() override;
    
    bool Initialize(const std::vector<std::string>& paths) override;
    void Run(std::function<void(FileEvent)> eventCallback) override;
    void Stop() override;
    bool AddPath(const std::string& path) override;
    bool RemovePath(const std::string& path) override;
    
    void SetPollInterval(std::chrono::milliseconds interval);
    
private:
    void PollLoop();
    void ScanDirectory(const std::string& path);
    void CheckFileChanges();
    FileWatcherInfo GetFileInfo(const std::string& path);
    void ProcessFileChange(const std::string& path, const FileWatcherInfo& oldInfo, const FileWatcherInfo& newInfo);
    void ProcessNewFile(const std::string& path, const FileWatcherInfo& info);
    void ProcessDeletedFile(const std::string& path);
    
    std::chrono::milliseconds m_pollInterval;
    std::vector<std::string> m_watchPaths;
    std::unordered_map<std::string, FileWatcherInfo> m_fileCache;
    std::function<void(FileEvent)> m_eventCallback;
    std::atomic<bool> m_running;
    std::thread m_pollThread;
    mutable std::mutex m_cacheMutex;
};