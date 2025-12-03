#pragma once
#include <string>
#include <vector>

namespace StackTrace
{
    struct Frame
    {
        std::string function;
        std::string file;
        void* address;
    };

    std::vector<Frame> Capture();
    std::string GetStackTrace();
    void InstallCrashHandler();
    void UninstallCrashHandler();


#if defined(__APPLE__) || defined(__linux__)
    const char* SignalToString(int sig);
#endif

#if defined(_WIN32)
    const char* ExceptionCodeToString(DWORD code);
#endif

}
