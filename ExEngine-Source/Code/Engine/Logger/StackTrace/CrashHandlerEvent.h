#pragma once
#include <string>
#include "../../File/FileManagement.h"

class CrashHandlerEvent{
public:
    inline static void CrashLog(std::string stackTrace){
        FileManagement::SaveFile(std::string("crash_report.txt"), stackTrace.c_str());
    };
};