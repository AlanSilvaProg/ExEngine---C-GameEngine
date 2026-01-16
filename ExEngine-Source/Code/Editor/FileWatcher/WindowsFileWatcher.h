#pragma once
#include "IPlatformWatcher.h"

#ifdef EXENGINE_WINDOWS
#include <windows.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>

struct WatchData {
    HANDLE directoryHandle;
    OVERLAPPED overlapped;
    std::string path;
    std::vector<BYTE> buffer;
    bool recursive;
    
    WatchData() : directoryHandle(INVALID_HANDLE_VALUE), recursive(true) {
        ZeroMemory(&overlapped, sizeof(overlapped));
        buffer.resize(64 * 1024); // 64KB buffer
    }
    
    ~WatchData() {
        if (directoryHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(directoryHandle);
        }
    }
};

class WindowsFileWatcher : public IPlatformWatcher {
public:
    WindowsFileWatcher();
    ~WindowsFileWatcher() override;
    
    bool Initialize(const std::vector<std::string>& paths) override;
    void Run(std::function<void(FileEvent)> eventCallback) override;
    void Stop() override;
    bool AddPath(const std::string& path) override;
    bool RemovePath(const std::string& path) override;
    
private:
    bool SetupWatch(const std::string& path);
    void ProcessEvents();
    FileEvent CreateFileEvent(const std::string& basePath, const FILE_NOTIFY_INFORMATION* info);
    std::string GetFullPath(const std::string& basePath, const std::wstring& relativePath);
    
    HANDLE m_completionPort;
    std::vector<std::unique_ptr<WatchData>> m_watchData;
    std::unordered_map<std::string, size_t> m_pathToIndex;
    std::function<void(FileEvent)> m_eventCallback;
    std::atomic<bool> m_running;
};

#endif // EXENGINE_WINDOWS