#pragma once
#include <functional>
#include <vector>
#include <string>
#include "FileEvent.h"

class IPlatformWatcher {
public:
    virtual ~IPlatformWatcher() = default;
    virtual bool Initialize(const std::vector<std::string>& paths) = 0;
    virtual void Run(std::function<void(FileEvent)> eventCallback) = 0;
    virtual void Stop() = 0;
    virtual bool AddPath(const std::string& path) = 0;
    virtual bool RemovePath(const std::string& path) = 0;
};