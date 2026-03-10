#include "PlatformWatcherFactory.h"
#include "PollingFileWatcher.h"
#include "../../Engine/Logger/Logger.h"

#ifdef EXENGINE_WINDOWS
#include "WindowsFileWatcher.h"
#elif defined(EXENGINE_MACOS)
#include "MacOSFileWatcher.h"
#elif defined(EXENGINE_LINUX)
#include "LinuxFileWatcher.h"
#endif

std::unique_ptr<IPlatformWatcher> PlatformWatcherFactory::Create() {
    std::unique_ptr<IPlatformWatcher> watcher;
    
#ifdef EXENGINE_WINDOWS
    Logger::Log("FileWatcher: Creating Windows file watcher");
    watcher = CreateWindowsWatcher();
#elif defined(EXENGINE_MACOS)
    Logger::Log("FileWatcher: Creating macOS file watcher");
    watcher = CreateMacOSWatcher();
#elif defined(EXENGINE_LINUX)
    Logger::Log("FileWatcher: Creating Linux file watcher");
    watcher = CreateLinuxWatcher();
#endif
    
    // Fallback to polling if platform-specific watcher failed
    if (!watcher) {
        Logger::Log("FileWatcher: Falling back to polling file watcher");
        watcher = CreatePollingWatcher();
    }
    
    return watcher;
}

std::unique_ptr<IPlatformWatcher> PlatformWatcherFactory::CreateWindowsWatcher() {
#ifdef EXENGINE_WINDOWS
    try {
        return std::make_unique<WindowsFileWatcher>();
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Failed to create Windows watcher: " + std::string(e.what()));
        return nullptr;
    }
#else
    return nullptr;
#endif
}

std::unique_ptr<IPlatformWatcher> PlatformWatcherFactory::CreateMacOSWatcher() {
#ifdef EXENGINE_MACOS
    try {
        return std::make_unique<MacOSFileWatcher>();
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Failed to create macOS watcher: " + std::string(e.what()));
        return nullptr;
    }
#else
    return nullptr;
#endif
}

std::unique_ptr<IPlatformWatcher> PlatformWatcherFactory::CreateLinuxWatcher() {
#ifdef EXENGINE_LINUX
    try {
        return std::make_unique<LinuxFileWatcher>();
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Failed to create Linux watcher: " + std::string(e.what()));
        return nullptr;
    }
#else
    return nullptr;
#endif
}

std::unique_ptr<IPlatformWatcher> PlatformWatcherFactory::CreatePollingWatcher() {
    try {
        return std::make_unique<PollingFileWatcher>();
    } catch (const std::exception& e) {
        Logger::LogError("FileWatcher: Failed to create polling watcher: " + std::string(e.what()));
        return nullptr;
    }
}