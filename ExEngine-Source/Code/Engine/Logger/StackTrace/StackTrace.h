#pragma once
#include <string>
#include <vector>

namespace StackTrace
{
    struct Frame
    {
        std::string function;      // Function name (demangled)
        std::string file;          // Source file path
        int line;                  // Line number (0 if not available)
        void* address;             // Memory address
        std::string module;        // Module/library name
        
        // Default constructor
        Frame() : line(0), address(nullptr) {}
    };

    // Symbol resolution functions
    bool ResolveSymbol(void* address, Frame& frame);
    bool ResolveLineInfo(void* address, Frame& frame);
    std::string FormatFrame(const Frame& frame, int index);
    std::string ParseFunctionFromBacktraceSymbol(const char* symbol);

    // Stack trace capture and formatting
    std::vector<Frame> Capture(bool resolveLines = false);
    std::string GetStackTrace();                    // Fast: no line info (for normal logging)
    std::string GetStackTraceWithLines();           // Slow: includes line info (for crashes)
    
    // Crash handler management
    void InstallCrashHandler();
    void UninstallCrashHandler();


#if defined(__APPLE__) || defined(__linux__)
    const char* SignalToString(int sig);
#endif

#if defined(_WIN32)
    const char* ExceptionCodeToString(DWORD code);
#endif

}
