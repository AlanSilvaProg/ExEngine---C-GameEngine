#pragma once
#include "IPlatformWatcher.h"

#ifdef EXENGINE_LINUX
#include <sys/inotify.h>
#include <unordered_map>
#include <atomic>
#include <vector>

class LinuxFileWatcher : public IPlatformWatcher {
public:
    LinuxFileWatcher();
    ~LinuxFileWatcher() override;
    
    bool Initialize(const std::vector<std::string>& paths) override;
    void Run(std::function<void(FileEvent)> eventCallback) override;
    void Stop() override;
    bool AddPath(const std::string& path) override;
    bool RemovePath(const std::string& path) override;
    
private:
    bool SetupWatch(const std::string& path);
    void ProcessEvents();
    void ProcessInotifyEvent(const struct inotify_event* event);
    FileEventType DetermineEventType(uint32_t mask);
    void AddRecursiveWatches(const std::string& path);
    
    int m_inotifyFd;
    std::unordered_map<int, std::string> m_watchDescriptors;
    std::unordered_map<std::string, int> m_pathToWd;
    std::function<void(FileEvent)> m_eventCallback;
    std::atomic<bool> m_running;
    std::vector<std::string> m_watchPaths;
};

#endif // EXENGINE_LINUX