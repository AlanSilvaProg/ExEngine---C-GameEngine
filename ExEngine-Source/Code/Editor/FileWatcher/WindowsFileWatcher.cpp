#include "WindowsFileWatcher.h"

#ifdef EXENGINE_WINDOWS
#include "FileWatcherError.h"
#include "../../Engine/Logger/Logger.h"
#include <filesystem>
#include <codecvt>
#include <locale>

WindowsFileWatcher::WindowsFileWatcher() 
    : m_completionPort(INVALID_HANDLE_VALUE), m_running(false) {
}

WindowsFileWatcher::~WindowsFileWatcher() {
    Stop();
    if (m_completionPort != INVALID_HANDLE_VALUE) {
        CloseHandle(m_completionPort);
    }
}

bool WindowsFileWatcher::Initialize(const std::vector<std::string>& paths) {
    // Create I/O completion port
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_completionPort == NULL) {
        throw FileWatcherException(FileWatcherError::PlatformApiFailure, 
            "Failed to create I/O completion port");
    }
    
    // Setup watches for all paths
    for (const auto& path : paths) {
        if (!SetupWatch(path)) {
            Logger::LogError("WindowsFileWatcher: Failed to setup watch for: " + path);
            return false;
        }
    }
    
    return true;
}

void WindowsFileWatcher::Run(std::function<void(FileEvent)> eventCallback) {
    m_eventCallback = eventCallback;
    m_running = true;
    
    // Start initial ReadDirectoryChangesW calls
    for (auto& watchData : m_watchData) {
        DWORD bytesReturned;
        BOOL result = ReadDirectoryChangesW(
            watchData->directoryHandle,
            watchData->buffer.data(),
            static_cast<DWORD>(watchData->buffer.size()),
            watchData->recursive ? TRUE : FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | 
            FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            &watchData->overlapped,
            NULL
        );
        
        if (!result && GetLastError() != ERROR_IO_PENDING) {
            Logger::LogError("WindowsFileWatcher: ReadDirectoryChangesW failed for: " + 
                      watchData->path);
        }
    }
    
    // Process events
    ProcessEvents();
}

void WindowsFileWatcher::Stop() {
    m_running = false;
    
    if (m_completionPort != INVALID_HANDLE_VALUE) {
        // Post a completion packet to wake up the waiting thread
        PostQueuedCompletionStatus(m_completionPort, 0, 0, NULL);
    }
}

bool WindowsFileWatcher::AddPath(const std::string& path) {
    if (m_pathToIndex.find(path) != m_pathToIndex.end()) {
        return true; // Already watching
    }
    
    return SetupWatch(path);
}

bool WindowsFileWatcher::RemovePath(const std::string& path) {
    auto it = m_pathToIndex.find(path);
    if (it == m_pathToIndex.end()) {
        return false; // Path not found
    }
    
    size_t index = it->second;
    
    // Cancel pending I/O operations
    CancelIo(m_watchData[index]->directoryHandle);
    
    // Remove from collections
    m_watchData.erase(m_watchData.begin() + index);
    m_pathToIndex.erase(it);
    
    // Update indices in map
    for (auto& pair : m_pathToIndex) {
        if (pair.second > index) {
            pair.second--;
        }
    }
    
    return true;
}

bool WindowsFileWatcher::SetupWatch(const std::string& path) {
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        return false;
    }
    
    // Convert path to wide string
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring widePath = converter.from_bytes(path);
    
    // Open directory handle
    HANDLE directoryHandle = CreateFileW(
        widePath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (directoryHandle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        Logger::LogError("WindowsFileWatcher: Failed to open directory: " + path + 
                  " Error: " + std::to_string(error));
        return false;
    }
    
    // Associate with completion port
    HANDLE result = CreateIoCompletionPort(directoryHandle, m_completionPort, 
                                          m_watchData.size(), 0);
    if (result == NULL) {
        CloseHandle(directoryHandle);
        return false;
    }
    
    // Create watch data
    auto watchData = std::make_unique<WatchData>();
    watchData->directoryHandle = directoryHandle;
    watchData->path = path;
    watchData->recursive = true;
    
    // Add to collections
    m_pathToIndex[path] = m_watchData.size();
    m_watchData.push_back(std::move(watchData));
    
    return true;
}

void WindowsFileWatcher::ProcessEvents() {
    while (m_running) {
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED overlapped;
        
        BOOL result = GetQueuedCompletionStatus(
            m_completionPort,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            1000 // 1 second timeout
        );
        
        if (!result) {
            DWORD error = GetLastError();
            if (error == WAIT_TIMEOUT) {
                continue; // Normal timeout, check if still running
            }
            if (error == ERROR_OPERATION_ABORTED) {
                break; // Shutdown requested
            }
            continue;
        }
        
        if (bytesTransferred == 0 || overlapped == NULL) {
            continue; // Shutdown signal
        }
        
        // Find the watch data for this completion
        WatchData* watchData = nullptr;
        for (auto& wd : m_watchData) {
            if (&wd->overlapped == overlapped) {
                watchData = wd.get();
                break;
            }
        }
        
        if (!watchData) {
            continue;
        }
        
        // Process the file notifications
        const FILE_NOTIFY_INFORMATION* info = 
            reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(watchData->buffer.data());
        
        while (info) {
            FileEvent event = CreateFileEvent(watchData->path, info);
            if (m_eventCallback) {
                m_eventCallback(event);
            }
            
            if (info->NextEntryOffset == 0) {
                break;
            }
            
            info = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<const BYTE*>(info) + info->NextEntryOffset);
        }
        
        // Restart the watch
        if (m_running) {
            ZeroMemory(&watchData->overlapped, sizeof(watchData->overlapped));
            DWORD bytesReturned;
            ReadDirectoryChangesW(
                watchData->directoryHandle,
                watchData->buffer.data(),
                static_cast<DWORD>(watchData->buffer.size()),
                watchData->recursive ? TRUE : FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | 
                FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned,
                &watchData->overlapped,
                NULL
            );
        }
    }
}

FileEvent WindowsFileWatcher::CreateFileEvent(const std::string& basePath, 
                                             const FILE_NOTIFY_INFORMATION* info) {
    FileEvent event;
    event.timestamp = std::chrono::system_clock::now();
    
    // Convert filename from wide string
    std::wstring filename(info->FileName, info->FileNameLength / sizeof(wchar_t));
    event.filePath = GetFullPath(basePath, filename);
    
    // Determine event type
    switch (info->Action) {
        case FILE_ACTION_ADDED:
            event.type = FileEventType::Created;
            break;
        case FILE_ACTION_REMOVED:
            event.type = FileEventType::Deleted;
            break;
        case FILE_ACTION_MODIFIED:
            event.type = FileEventType::Modified;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
        case FILE_ACTION_RENAMED_NEW_NAME:
            event.type = FileEventType::Renamed;
            break;
        default:
            event.type = FileEventType::Modified;
            break;
    }
    
    // Get file size for created/modified events
    if (event.type == FileEventType::Created || event.type == FileEventType::Modified) {
        try {
            if (std::filesystem::exists(event.filePath)) {
                event.fileSize = std::filesystem::file_size(event.filePath);
            }
        } catch (const std::exception&) {
            event.fileSize = 0;
        }
    }
    
    return event;
}

std::string WindowsFileWatcher::GetFullPath(const std::string& basePath, 
                                           const std::wstring& relativePath) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string relativePathStr = converter.to_bytes(relativePath);
    
    std::filesystem::path fullPath = std::filesystem::path(basePath) / relativePathStr;
    return fullPath.string();
}

#endif // EXENGINE_WINDOWS