#include "StackTrace.h"
#include "../../Core/Serializer/Demangle.h"
#include "CrashHandlerEvent.h"
#include <iostream>
#include <sstream>

#if defined(__APPLE__) || defined(__linux__)

    #include <execinfo.h>
    #include <cxxabi.h>
    #include <signal.h>
    #include <dlfcn.h>

#elif defined(_WIN32)

    #include <windows.h>
    #include <dbghelp.h>

#endif

namespace StackTrace
{

#if defined(__APPLE__) || defined(__linux__)

    // Helper function to extract function name from backtrace_symbols output
    // Typical format: "2   libEngine.dylib  0x000000010abcd123 _ZN6Logger3LogEv + 45"
    std::string ParseFunctionFromBacktraceSymbol(const char* symbol)
    {
        if (!symbol)
            return "";
        
        std::string str(symbol);
        
        // Find the mangled symbol between address and " + " offset
        // Format varies by platform but generally: module address symbol + offset
        
        // Look for " + " which marks the offset
        size_t plusPos = str.find(" + ");
        if (plusPos == std::string::npos)
            return "";
        
        // Work backwards from " + " to find the symbol
        size_t symbolEnd = plusPos;
        
        // Skip whitespace before " + "
        while (symbolEnd > 0 && (str[symbolEnd - 1] == ' ' || str[symbolEnd - 1] == '\t'))
            symbolEnd--;
        
        if (symbolEnd == 0)
            return "";
        
        // Find the start of the symbol (after the address)
        size_t symbolStart = symbolEnd;
        while (symbolStart > 0 && str[symbolStart - 1] != ' ' && str[symbolStart - 1] != '\t')
            symbolStart--;
        
        if (symbolStart >= symbolEnd)
            return "";
        
        std::string mangledName = str.substr(symbolStart, symbolEnd - symbolStart);
        
        // Demangle the symbol
        return Demangle(mangledName.c_str());
    }

    // Resolve line information for a given address (optional, requires debug symbols)
    // Uses atos on macOS or addr2line on Linux
    bool ResolveLineInfo(void* address, Frame& frame)
    {
        #ifdef __APPLE__
        // Use atos on macOS
        Dl_info info;
        if (dladdr(address, &info) == 0 || !info.dli_fname)
            return false;
        
        // Build atos command: atos -o <binary> -l <load_address> <address>
        std::ostringstream cmd;
        cmd << "atos -o " << info.dli_fname 
            << " -l " << info.dli_fbase 
            << " " << address 
            << " 2>/dev/null";
        
        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe)
            return false;
        
        char buffer[512];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            result += buffer;
        
        pclose(pipe);
        
        // Parse atos output format: "function (in module) (file:line)"
        // Example: "main (in myapp) (main.cpp:42)"
        size_t fileStart = result.rfind('(');
        size_t fileEnd = result.rfind(')');
        
        if (fileStart != std::string::npos && fileEnd != std::string::npos && fileStart < fileEnd)
        {
            std::string fileInfo = result.substr(fileStart + 1, fileEnd - fileStart - 1);
            
            // Split by ':' to get file and line
            size_t colonPos = fileInfo.rfind(':');
            if (colonPos != std::string::npos)
            {
                frame.file = fileInfo.substr(0, colonPos);
                
                // Parse line number
                std::string lineStr = fileInfo.substr(colonPos + 1);
                try {
                    frame.line = std::stoi(lineStr);
                    return true;
                } catch (...) {
                    return false;
                }
            }
        }
        
        return false;
        
        #elif defined(__linux__)
        // Use addr2line on Linux
        Dl_info info;
        if (dladdr(address, &info) == 0 || !info.dli_fname)
            return false;
        
        // Calculate offset from base address
        uintptr_t offset = (uintptr_t)address - (uintptr_t)info.dli_fbase;
        
        // Build addr2line command
        std::ostringstream cmd;
        cmd << "addr2line -e " << info.dli_fname 
            << " -f -C 0x" << std::hex << offset 
            << " 2>/dev/null";
        
        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe)
            return false;
        
        char buffer[512];
        std::string functionLine;
        std::string fileLine;
        
        // addr2line outputs two lines: function name, then file:line
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            functionLine = buffer;
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            fileLine = buffer;
        
        pclose(pipe);
        
        // Parse file:line format
        if (!fileLine.empty())
        {
            // Remove trailing newline
            if (fileLine.back() == '\n')
                fileLine.pop_back();
            
            size_t colonPos = fileLine.rfind(':');
            if (colonPos != std::string::npos)
            {
                frame.file = fileLine.substr(0, colonPos);
                
                // Parse line number
                std::string lineStr = fileLine.substr(colonPos + 1);
                try {
                    frame.line = std::stoi(lineStr);
                    return true;
                } catch (...) {
                    return false;
                }
            }
        }
        
        return false;
        #else
        return false;
        #endif
    }

    // Resolve symbol information for a given address using dladdr
    bool ResolveSymbol(void* address, Frame& frame)
    {
        frame.address = address;
        
        Dl_info info;
        if (dladdr(address, &info) != 0)
        {
            // Extract module name
            if (info.dli_fname)
            {
                frame.module = info.dli_fname;
                
                // Extract just the filename from the full path
                size_t lastSlash = frame.module.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                    frame.module = frame.module.substr(lastSlash + 1);
            }
            
            // Extract function name
            if (info.dli_sname)
            {
                frame.function = Demangle(info.dli_sname);
                return true;
            }
        }
        
        // Fallback: try to extract from backtrace_symbols
        char** symbols = backtrace_symbols(&address, 1);
        if (symbols)
        {
            frame.function = ParseFunctionFromBacktraceSymbol(symbols[0]);
            free(symbols);
            
            if (!frame.function.empty())
                return true;
        }
        
        // If all else fails, mark as unknown
        frame.function = "<unknown>";
        return false;
    }

    std::vector<Frame> Capture(bool resolveLines)
    {
        std::vector<Frame> result;

        void* callstack[128];
        int frameCount = backtrace(callstack, 128);

        for (int i = 0; i < frameCount; i++)
        {
            Frame frame;
            
            // Resolve symbol information using enhanced resolution
            if (ResolveSymbol(callstack[i], frame))
            {
                // Try to get file and line information (optional, requires debug symbols)
                // Only resolve if explicitly requested (e.g., for crash handlers)
                if (resolveLines)
                {
                    ResolveLineInfo(callstack[i], frame);
                }
                result.push_back(frame);
            }
            else
            {
                // Even if symbol resolution fails, include the frame with address
                // ResolveSymbol already sets function to "<unknown>" on failure
                frame.address = callstack[i];
                frame.function = "<unknown>";
                result.push_back(frame);
            }
        }

        return result;
    }

    void CrashHandler(int sig)
    {
        // Capture frames directly to exclude crash handler frames
        // Enable line resolution for crash handlers (performance is not critical during crashes)
        auto frames = Capture(true);
        
        // Build the crash log with signal information
        std::string stack = "Crash reason: ";
        stack += SignalToString(sig);
        stack += "\n";
        
        // Skip the first few frames that belong to the crash handler itself
        // Typically: CrashHandler -> signal handler -> actual crash location
        // We want to start from the actual crash location
        size_t skipFrames = 0;
        for (size_t i = 0; i < frames.size(); i++)
        {
            // Skip frames that contain "CrashHandler" or signal-related functions
            if (frames[i].function.find("CrashHandler") != std::string::npos ||
                frames[i].function.find("_sigtramp") != std::string::npos ||
                frames[i].function.find("__restore_rt") != std::string::npos)
            {
                skipFrames = i + 1;
            }
            else
            {
                // Once we hit a non-handler frame, stop skipping
                break;
            }
        }
        
        // Format the remaining frames
        for (size_t i = skipFrames; i < frames.size(); i++)
        {
            stack += FormatFrame(frames[i], static_cast<int>(i - skipFrames));
            stack += "\n";
        }

        CrashHandlerEvent::CrashLog(stack);

        exit(1);
    }

    void InstallCrashHandler()
    {
        signal(SIGSEGV, CrashHandler);
        signal(SIGABRT, CrashHandler);
        signal(SIGFPE,  CrashHandler);
        signal(SIGILL,  CrashHandler);
        signal(SIGTERM, CrashHandler);
    }

    void UninstallCrashHandler()
    {
        signal(SIGSEGV, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
        signal(SIGFPE,  SIG_DFL);
        signal(SIGILL,  SIG_DFL);
        signal(SIGTERM, SIG_DFL);
    }

    const char* SignalToString(int sig)
    {
        switch (sig)
        {
            case SIGSEGV: return "SIGSEGV (Segmentation Fault)";
            case SIGABRT: return "SIGABRT (Abort)";
            case SIGFPE:  return "SIGFPE (Floating Point Error)";
            case SIGILL:  return "SIGILL (Illegal Instruction)";
            case SIGBUS:  return "SIGBUS (Bus Error)";
            case SIGTERM: return "SIGTERM (Terminated)";
            default:      return "Unknown signal";
        }
    }

#endif

#if defined(_WIN32)

    // Resolve symbol information for Windows using DbgHelp
    bool ResolveSymbol(void* address, Frame& frame)
    {
        frame.address = address;
        
        HANDLE process = GetCurrentProcess();
        
        // Allocate symbol info structure
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);
        if (!symbol)
        {
            frame.function = "<unknown>";
            return false;
        }
        
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        
        // Try to resolve the symbol
        if (SymFromAddr(process, (DWORD64)address, 0, symbol))
        {
            frame.function = symbol->Name;
            free(symbol);
            return true;
        }
        
        // Failed to resolve
        free(symbol);
        frame.function = "<unknown>";
        return false;
    }

    // Resolve line information for Windows using DbgHelp
    bool ResolveLineInfo(void* address, Frame& frame)
    {
        HANDLE process = GetCurrentProcess();
        
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        
        DWORD displacement = 0;
        if (SymGetLineFromAddr64(process, (DWORD64)address, &displacement, &line))
        {
            frame.file = line.FileName;
            frame.line = line.LineNumber;
            return true;
        }
        
        return false;
    }

    std::vector<Frame> Capture(bool resolveLines = false)
    {
        std::vector<Frame> result;

        void* stack[128];
        USHORT frames = CaptureStackBackTrace(0, 128, stack, NULL);

        HANDLE process = GetCurrentProcess();
        
        // Initialize DbgHelp for symbol resolution
        // SymInitialize is idempotent - safe to call multiple times
        SymInitialize(process, NULL, TRUE);

        for (USHORT i = 0; i < frames; i++)
        {
            Frame frame;
            
            // Resolve symbol information (function name)
            if (ResolveSymbol(stack[i], frame))
            {
                // Try to get file and line information (requires debug symbols)
                // Only resolve if explicitly requested (e.g., for crash handlers)
                if (resolveLines)
                {
                    ResolveLineInfo(stack[i], frame);
                }
                result.push_back(frame);
            }
            else
            {
                // Even if symbol resolution fails, include the frame with address
                frame.address = stack[i];
                frame.function = "<unknown>";
                result.push_back(frame);
            }
        }

        return result;
    }

    LONG WINAPI WinCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
    {
        DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;

        // Capture frames directly to exclude crash handler frames
        // Enable line resolution for crash handlers (performance is not critical during crashes)
        auto frames = Capture(true);
        
        // Build the crash log with exception information
        std::string stack = "Crash reason: ";
        stack += ExceptionCodeToString(code);
        stack += "\n";
        
        // Skip the first few frames that belong to the crash handler itself
        // Typically: WinCrashHandler -> exception dispatcher -> actual crash location
        // We want to start from the actual crash location
        size_t skipFrames = 0;
        for (size_t i = 0; i < frames.size(); i++)
        {
            // Skip frames that contain "WinCrashHandler" or Windows exception handling functions
            if (frames[i].function.find("WinCrashHandler") != std::string::npos ||
                frames[i].function.find("UnhandledExceptionFilter") != std::string::npos ||
                frames[i].function.find("RtlUserThreadStart") != std::string::npos ||
                frames[i].function.find("_C_specific_handler") != std::string::npos)
            {
                skipFrames = i + 1;
            }
            else
            {
                // Once we hit a non-handler frame, stop skipping
                break;
            }
        }
        
        // Format the remaining frames
        for (size_t i = skipFrames; i < frames.size(); i++)
        {
            stack += FormatFrame(frames[i], static_cast<int>(i - skipFrames));
            stack += "\n";
        }

        CrashHandlerEvent::CrashLog(stack);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void InstallCrashHandler()
    {
        SetUnhandledExceptionFilter(WinCrashHandler);
    }

    void UninstallCrashHandler()
    {
        SetUnhandledExceptionFilter(nullptr);
    }
    
    const char* ExceptionCodeToString(DWORD code)
    {
        switch (code)
        {
            case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION (Invalid memory read/write)";
            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
            case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
            case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
            case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
            case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
            case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
            case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
            case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
            case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
            case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
            case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
            case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
            case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
            case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
            case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
            case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
            case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
            default:                                 return "Unknown Windows exception";
        }
    }

#endif

    // Format a single frame into a readable string
    // Pattern: "index: function [file:line] (address)"
    // Handles missing optional information gracefully
    // Extracts basename for long file paths
    // Never truncates function names
    std::string FormatFrame(const Frame& frame, int index)
    {
        std::ostringstream oss;
        // Function name (never truncated, always included)
        oss << frame.function;
        
        // File and line information (optional)
        if (!frame.file.empty() && frame.line > 0)
        {
            // Extract basename from file path for readability
            std::string filename = frame.file;
            size_t lastSlash = filename.find_last_of("/\\");
            if (lastSlash != std::string::npos)
                filename = filename.substr(lastSlash + 1);
            
            oss << " [" << filename << ":" << frame.line << "]";
        }
        else if (!frame.file.empty())
        {
            // File available but no line number
            std::string filename = frame.file;
            size_t lastSlash = filename.find_last_of("/\\");
            if (lastSlash != std::string::npos)
                filename = filename.substr(lastSlash + 1);
            
            oss << " [" << filename << "]";
        }
        // If no file info, omit the brackets entirely
        
        // Address
        // oss << " (" << frame.address << ")";
        
        return oss.str();
    }

    std::string GetStackTrace()
    {
        // Fast: no line resolution for normal logging
        auto frames = Capture(false);
        std::string stack;
        
        // Format each frame using FormatFrame() with sequential indices starting from 0
        // skipping log sequence by starting from 3
        for (size_t i = 3; i < frames.size(); i++)
        {
            stack += FormatFrame(frames[i], static_cast<int>(i));
            stack += "\n";
        }

        return stack;
    }

    std::string GetStackTraceWithLines()
    {
        // Slower: includes line resolution with caching (for crash handlers)
        auto frames = Capture(true);
        std::string stack;
        
        // Format each frame using FormatFrame() with sequential indices starting from 0
        for (size_t i = 0; i < frames.size(); i++)
        {
            stack += FormatFrame(frames[i], static_cast<int>(i));
            stack += "\n";
        }

        return stack;
    }

}
