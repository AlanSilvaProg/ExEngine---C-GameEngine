#pragma once
#include <memory>
#include "IPlatformWatcher.h"

class PlatformWatcherFactory {
public:
    static std::unique_ptr<IPlatformWatcher> Create();
    
private:
    static std::unique_ptr<IPlatformWatcher> CreateWindowsWatcher();
    static std::unique_ptr<IPlatformWatcher> CreateMacOSWatcher();
    static std::unique_ptr<IPlatformWatcher> CreateLinuxWatcher();
    static std::unique_ptr<IPlatformWatcher> CreatePollingWatcher();
};