#include "LinuxFileWatcher.h"

#ifdef EXENGINE_LINUX
#include "FileWatcherError.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <cstring>

LinuxFileWatcher::LinuxFileWatcher() 
    : m_inotifyFd(-1), m_running(false) {
}

LinuxFileWatcher::~LinuxFileWatcher() {
    Stop();
    if (m_inotifyFd != -1) {
        close(m_inotifyFd);
    }
}

bool LinuxFileWatcher::Initialize(const std::vector<std::string>& paths) {
    // Initialize inotify
    m_inotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (m_inotifyFd == -1) {
        throw FileWatcherException(FileWatcherError::PlatformApiFailure, 
            "Failed to initialize inotify: " + std::string(strerror(errno)));
    }
    
    m_watchPaths = paths;
    
    // Setup watches for all paths
    for (const auto& path : paths) {
        if (!SetupWatch(path)) {
            Logger::LogError("LinuxFileWatcher: Failed to setup watch for: " + path);
            return false;
        }
    }
    
    return true;
}

void LinuxFileWatcher::Run(std::function<void(FileEvent)> eventCallback) {
    m_eventCallback = eventCallback;
    m_running = true;
    
    ProcessEvents();
}

void LinuxFileWatcher::Stop() {
    m_running = false;
    
    // Remove all watches
    for (const auto& pair : m_watchDescriptors) {
        inotify_rm_watch(m_inotifyFd, pair.first);
    }
    
    m_watchDescriptors.clear();
    m_pathToWd.clear();
}

bool LinuxFileWatcher::AddPath(const std::string& path) {
    if (m_pathToWd.find(path) != m_pathToWd.end()) {
        return true; // Already watching
    }
    
    m_watchPaths.push_back(path);
    return SetupWatch(path);
}

bool LinuxFileWatcher::RemovePath(const std::string& path) {
    auto it = m_pathToWd.find(path);
    if (it == m_pathToWd.end()) {
        return false; // Path not found
    }
    
    int wd = it->second;
    
    // Remove inotify watch
    if (inotify_rm_watch(m_inotifyFd, wd) == -1) {
        Logger::LogError("LinuxFileWatcher: Failed to remove watch: " + std::string(strerror(errno)));
        return false;
    }
    
    // Remove from maps
    m_watchDescriptors.erase(wd);
    m_pathToWd.erase(it);
    
    // Remove from watch paths
    auto pathIt = std::find(m_watchPaths.begin(), m_watchPaths.end(), path);
    if (pathIt != m_watchPaths.end()) {
        m_watchPaths.erase(pathIt);
    }
    
    return true;
}

bool LinuxFileWatcher::SetupWatch(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    // Add recursive watches if it's a directory
    if (std::filesystem::is_directory(path)) {
        AddRecursiveWatches(path);
    } else {
        // Watch the parent directory for file changes
        std::filesystem::path parentPath = std::filesystem::path(path).parent_path();
        AddRecursiveWatches(parentPath.string());
    }
    
    return true;
}

void LinuxFileWatcher::AddRecursiveWatches(const std::string& path) {
    try {
        // Add watch for this directory
        uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF;
        
        int wd = inotify_add_watch(m_inotifyFd, path.c_str(), mask);
        if (wd == -1) {
            Logger::LogError("LinuxFileWatcher: Failed to add watch for " + path + 
                      ": " + std::string(strerror(errno)));
            return;
        }
        
        m_watchDescriptors[wd] = path;
        m_pathToWd[path] = wd;
        
        // Recursively add watches for subdirectories
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_directory()) {
                std::string subPath = entry.path().string();
                
                // Skip if already watching
                if (m_pathToWd.find(subPath) != m_pathToWd.end()) {
                    continue;
                }
                
                int subWd = inotify_add_watch(m_inotifyFd, subPath.c_str(), mask);
                if (subWd != -1) {
                    m_watchDescriptors[subWd] = subPath;
                    m_pathToWd[subPath] = subWd;
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::LogError("LinuxFileWatcher: Error setting up recursive watches: " + std::string(e.what()));
    }
}

void LinuxFileWatcher::ProcessEvents() {
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    
    while (m_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_inotifyFd, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(m_inotifyFd + 1, &readfds, NULL, NULL, &timeout);
        
        if (result == -1) {
            if (errno == EINTR) {
                continue;
            }
            Logger::LogError("LinuxFileWatcher: select() failed: " + std::string(strerror(errno)));
            break;
        }
        
        if (result == 0) {
            continue; // Timeout, check if still running
        }
        
        if (FD_ISSET(m_inotifyFd, &readfds)) {
            ssize_t length = read(m_inotifyFd, buffer, bufferSize);
            if (length == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }
                Logger::LogError("LinuxFileWatcher: read() failed: " + std::string(strerror(errno)));
                break;
            }
            
            // Process all events in the buffer
            ssize_t offset = 0;
            while (offset < length) {
                const struct inotify_event* event = 
                    reinterpret_cast<const struct inotify_event*>(buffer + offset);
                
                ProcessInotifyEvent(event);
                
                offset += sizeof(struct inotify_event) + event->len;
            }
        }
    }
}

void LinuxFileWatcher::ProcessInotifyEvent(const struct inotify_event* event) {
    if (!m_eventCallback) {
        return;
    }
    
    // Find the directory path for this watch descriptor
    auto it = m_watchDescriptors.find(event->wd);
    if (it == m_watchDescriptors.end()) {
        return;
    }
    
    std::string dirPath = it->second;
    std::string filePath = dirPath;
    
    // Add filename if provided
    if (event->len > 0 && event->name[0] != '\0') {
        filePath = (std::filesystem::path(dirPath) / event->name).string();
    }
    
    FileEvent fileEvent;
    fileEvent.filePath = filePath;
    fileEvent.timestamp = std::chrono::system_clock::now();
    fileEvent.type = DetermineEventType(event->mask);
    
    // Get file size for created/modified events
    if (fileEvent.type == FileEventType::Created || fileEvent.type == FileEventType::Modified) {
        try {
            if (std::filesystem::exists(filePath)) {
                fileEvent.fileSize = std::filesystem::file_size(filePath);
                
                // If a new directory was created, add watches for it
                if (std::filesystem::is_directory(filePath) && 
                    fileEvent.type == FileEventType::Created) {
                    AddRecursiveWatches(filePath);
                }
            }
        } catch (const std::exception&) {
            fileEvent.fileSize = 0;
        }
    }
    
    m_eventCallback(fileEvent);
}

FileEventType LinuxFileWatcher::DetermineEventType(uint32_t mask) {
    if (mask & (IN_CREATE | IN_MOVED_TO)) {
        return FileEventType::Created;
    } else if (mask & (IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM)) {
        return FileEventType::Deleted;
    } else if (mask & (IN_MOVED_FROM | IN_MOVED_TO)) {
        return FileEventType::Renamed;
    } else if (mask & IN_MODIFY) {
        return FileEventType::Modified;
    }
    
    return FileEventType::Modified; // Default fallback
}

#endif // EXENGINE_LINUX