#include "StackTrace.h"
#include "../../Core/Serializer/Demangle.h"
#include "CrashHandlerEvent.h"
#include <iostream>
#include <sstream>

#if defined(__APPLE__) || defined(__linux__)

    #include <execinfo.h>
    #include <cxxabi.h>
    #include <signal.h>

#elif defined(_WIN32)

    #include <windows.h>
    #include <dbghelp.h>

#endif

namespace StackTrace
{

#if defined(__APPLE__) || defined(__linux__)

    std::vector<Frame> Capture()
    {
        std::vector<Frame> result;

        void* callstack[128];
        int frameCount = backtrace(callstack, 128);

        char** symbols = backtrace_symbols(callstack, frameCount);

        for (int i = 0; i < frameCount; i++)
        {
            Frame f;
            f.address = callstack[i];
            f.function = symbols[i]; // será demangled depois
            
            // Demangle
            f.function = Demangle(f.function.c_str());

            result.push_back(f);
        }

        free(symbols);
        return result;
    }

    void CrashHandler(int sig)
    {
        std::string stack = "Crash reason: ";
        stack += SignalToString(sig);
        stack += "\n";
        stack += GetStackTrace();

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

    std::vector<Frame> Capture()
    {
        std::vector<Frame> result;

        void* stack[128];
        USHORT frames = CaptureStackBackTrace(0, 128, stack, NULL);

        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);

        SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256, 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (USHORT i = 0; i < frames; i++)
        {
            SymFromAddr(process, (DWORD64)stack[i], 0, symbol);

            Frame f;
            f.function = symbol->Name;
            f.address = stack[i];
            result.push_back(f);
        }

        free(symbol);
        return result;
    }

    LONG WINAPI WinCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
    {
        DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;

        std::string stack += "Crash reason: ";
        stack += ExceptionCodeToString(code);
        stack += "\n";
        stack += GetStackTrace();

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

    std::string GetStackTrace()
    {
        auto frames = Capture();
        std::string stack;
        
        for (int i = (int)frames.size() - 1; i >= 0; i--)
        {
            std::ostringstream oss;
            oss << frames[i].address; // imprime estilo 0x7ffee...
            
            stack += std::to_string(i) + ": " + frames[i].function 
                + " [" + oss.str() + "]\n";
        }

        return stack;
    }
}
