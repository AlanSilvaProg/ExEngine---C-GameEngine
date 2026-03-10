#pragma once
#include "LogEntry.h"
#include <string>
#include <vector>

class Logger{
private:
static std::vector<std::shared_ptr<LogEntry>> logEntries;
static const std::string logPrefix;

static std::string GetCurrentTime();
static void GetComposition(std::string& messageComposition);

static void RegisterEntry(std::shared_ptr<LogEntry> logEntry);

public:
static void Log(std::string message);
static void Log(std::string message, LogType logType);
static void LogWarning(std::string message);
static void LogError(std::string message);

inline static const std::vector<std::shared_ptr<LogEntry>>& GetAllLogEntries() { return logEntries; };
};