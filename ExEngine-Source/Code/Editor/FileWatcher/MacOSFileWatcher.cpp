#include "MacOSFileWatcher.h"

#if defined(EXENGINE_MACOS) && !defined(EXENGINE_DISABLE_MACOS_FILEWATCHER)
#include "FileWatcherError.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>
#include <CoreFoundation/CoreFoundation.h>

MacOSFileWatcher::MacOSFileWatcher() 
    : m_eventStream(nullptr), m_runLoop(nullptr), m_running(false) {
}

MacOSFileWatcher::~MacOSFileWatcher() {
    Stop();
}

bool MacOSFileWatcher::Initialize(const std::vector<std::string>& paths) {
    m_watchPaths = paths;
    
    // Validate all paths exist
    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            Logger::LogError("MacOSFileWatcher: Path does not exist: " + path);
            return false;
        }
    }
    
    return true;
}

void MacOSFileWatcher::Run(std::function<void(FileEvent)> eventCallback) {
    m_eventCallback = eventCallback;
    m_running = true;
    
    // Create CFArray of paths to watch
    CFMutableArrayRef pathsArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    
    for (const auto& path : m_watchPaths) {
        CFStringRef pathString = CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
        CFArrayAppendValue(pathsArray, pathString);
        CFRelease(pathString);
    }
    
    // Create FSEventStream
    FSEventStreamContext context = {0, this, NULL, NULL, NULL};
    
    m_eventStream = FSEventStreamCreate(
        NULL,
        &MacOSFileWatcher::EventStreamCallback,
        &context,
        pathsArray,
        kFSEventStreamEventIdSinceNow,
        0.1, // 100ms latency
        kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes
    );
    
    CFRelease(pathsArray);
    
    if (!m_eventStream) {
        throw FileWatcherException(FileWatcherError::PlatformApiFailure, 
            "Failed to create FSEventStream");
    }
    
    // Start run loop in separate thread
    m_runLoopThread = std::thread([this]() {
        m_runLoop = CFRunLoopGetCurrent();
        
        FSEventStreamScheduleWithRunLoop(m_eventStream, m_runLoop, kCFRunLoopDefaultMode);
        
        if (!FSEventStreamStart(m_eventStream)) {
            Logger::LogError("MacOSFileWatcher: Failed to start FSEventStream");
            return;
        }
        
        // Run the event loop
        while (m_running) {
            CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
        }
        
        FSEventStreamStop(m_eventStream);
        FSEventStreamUnscheduleFromRunLoop(m_eventStream, m_runLoop, kCFRunLoopDefaultMode);
    });
}

void MacOSFileWatcher::Stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    
    if (m_runLoop) {
        CFRunLoopStop(m_runLoop);
    }
    
    if (m_runLoopThread.joinable()) {
        m_runLoopThread.join();
    }
    
    if (m_eventStream) {
        FSEventStreamRelease(m_eventStream);
        m_eventStream = nullptr;
    }
    
    m_runLoop = nullptr;
}

bool MacOSFileWatcher::AddPath(const std::string& path) {
    // For FSEvents, we need to recreate the stream with new paths
    // This is a limitation of the FSEvents API
    auto it = std::find(m_watchPaths.begin(), m_watchPaths.end(), path);
    if (it != m_watchPaths.end()) {
        return true; // Already watching
    }
    
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    m_watchPaths.push_back(path);
    
    // If running, we need to restart with new paths
    if (m_running) {
        bool wasRunning = m_running;
        auto callback = m_eventCallback;
        
        Stop();
        
        if (wasRunning) {
            Run(callback);
        }
    }
    
    return true;
}

bool MacOSFileWatcher::RemovePath(const std::string& path) {
    auto it = std::find(m_watchPaths.begin(), m_watchPaths.end(), path);
    if (it == m_watchPaths.end()) {
        return false;
    }
    
    m_watchPaths.erase(it);
    
    // If running, we need to restart with new paths
    if (m_running) {
        bool wasRunning = m_running;
        auto callback = m_eventCallback;
        
        Stop();
        
        if (wasRunning && !m_watchPaths.empty()) {
            Run(callback);
        }
    }
    
    return true;
}

void MacOSFileWatcher::EventStreamCallback(ConstFSEventStreamRef streamRef,
                                          void* clientCallBackInfo,
                                          size_t numEvents,
                                          void* eventPaths,
                                          const FSEventStreamEventFlags eventFlags[],
                                          const FSEventStreamEventId eventIds[]) {
    MacOSFileWatcher* watcher = static_cast<MacOSFileWatcher*>(clientCallBackInfo);
    CFArrayRef paths = static_cast<CFArrayRef>(eventPaths);
    
    for (size_t i = 0; i < numEvents; i++) {
        CFStringRef pathRef = static_cast<CFStringRef>(CFArrayGetValueAtIndex(paths, i));
        
        // Convert CFString to std::string
        CFIndex length = CFStringGetLength(pathRef);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
        std::vector<char> buffer(maxSize);
        
        if (CFStringGetCString(pathRef, buffer.data(), maxSize, kCFStringEncodingUTF8)) {
            std::string path(buffer.data());
            watcher->ProcessEvent(path, eventFlags[i]);
        }
    }
}

void MacOSFileWatcher::ProcessEvent(const std::string& path, FSEventStreamEventFlags flags) {
    if (!m_eventCallback) {
        return;
    }
    
    FileEvent event;
    event.filePath = path;
    event.timestamp = std::chrono::system_clock::now();
    event.type = DetermineEventType(flags);
    
    // Get file size for created/modified events
    if (event.type == FileEventType::Created || event.type == FileEventType::Modified) {
        try {
            if (std::filesystem::exists(path)) {
                event.fileSize = std::filesystem::file_size(path);
            }
        } catch (const std::exception&) {
            event.fileSize = 0;
        }
    }
    
    m_eventCallback(event);
}

FileEventType MacOSFileWatcher::DetermineEventType(FSEventStreamEventFlags flags) {
    if (flags & kFSEventStreamEventFlagItemCreated) {
        return FileEventType::Created;
    } else if (flags & kFSEventStreamEventFlagItemRemoved) {
        return FileEventType::Deleted;
    } else if (flags & kFSEventStreamEventFlagItemRenamed) {
        return FileEventType::Renamed;
    } else if (flags & (kFSEventStreamEventFlagItemModified | 
                       kFSEventStreamEventFlagItemInodeMetaMod |
                       kFSEventStreamEventFlagItemChangeOwner |
                       kFSEventStreamEventFlagItemXattrMod)) {
        return FileEventType::Modified;
    }
    
    return FileEventType::Modified; // Default fallback
}

#endif // EXENGINE_MACOS && !EXENGINE_DISABLE_MACOS_FILEWATCHER