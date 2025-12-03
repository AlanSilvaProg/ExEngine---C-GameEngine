#pragma once
#include <string>

enum LogType{
    LOG,
    WARNING,
    ERROR
};

struct LogEntry{
public:
    LogType logType;
    std::string message;
    std::string stackTrace;
};