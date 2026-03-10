#pragma once
#include "IPlatformWatcher.h"

#ifdef EXENGINE_MACOS
#include <CoreServices/CoreServices.h>
#include <vector>
#include <atomic>
#include <thread>

class MacOSFileWatcher : public IPlatformWatcher {
public:
    MacOSFileWatcher();
    ~MacOSFileWatcher() override;
    
    bool Initialize(const std::vector<std::string>& paths) override;
    void Run(std::function<void(FileEvent)> eventCallback) override;
    void Stop() override;
    bool AddPath(const std::string& path) override;
    bool RemovePath(const std::string& path) override;
    
private:
    static void EventStreamCallback(ConstFSEventStreamRef streamRef,
                                   void* clientCallBackInfo,
                                   size_t numEvents,
                                   void* eventPaths,
                                   const FSEventStreamEventFlags eventFlags[],
                                   const FSEventStreamEventId eventIds[]);
    
    void ProcessEvent(const std::string& path, FSEventStreamEventFlags flags);
    FileEventType DetermineEventType(FSEventStreamEventFlags flags);
    
    FSEventStreamRef m_eventStream;
    CFRunLoopRef m_runLoop;
    std::vector<std::string> m_watchPaths;
    std::function<void(FileEvent)> m_eventCallback;
    std::atomic<bool> m_running;
    std::thread m_runLoopThread;
};

#endif // EXENGINE_MACOS