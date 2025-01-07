#pragma once
#include <string>
#include <vector>

enum LogType{
    LOG,
    WARNING,
    ERROR
};

struct LogEntry{
    LogType logType;
    std::string message;
};

class Logger{
private:
static std::vector<LogEntry> logEntries;
static const std::string logPrefix;

static std::string GetCurrentTime();
static void GetComposition(std::string& messageComposition);

public:
static void Log(std::string message);
static void Log(std::string message, LogType logType);
static void LogWarning(std::string message);
static void LogError(std::string message);
};