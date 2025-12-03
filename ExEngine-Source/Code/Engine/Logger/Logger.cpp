#include "Logger.h"
#include "LoggerNotifier.h"
#include "StackTrace/StackTrace.h"
#include <iostream>
#include <stdlib.h> 
#include <chrono>
#include <ctime>

std::vector<std::shared_ptr<LogEntry>> Logger::logEntries;
const std::string Logger::logPrefix = "[ExEngine]-";

std::string Logger::GetCurrentTime(){
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%d-%b-%Y %H:%M:%S", localtime(&now));
    return std::string("[") + buffer + "]";
};

void Logger::GetComposition(std::string& messageComposition){
    messageComposition += GetCurrentTime();
    messageComposition += logPrefix;
};

void Logger::Log(std::string message, LogType logType){
    switch (logType)
    {
        case LogType::LOG:
            Log(message);
            break;
        case LogType::WARNING:
            LogWarning(message);
            break;
        case LogType::ERROR:
            LogError(message);
            break;
    }
};

void Logger::Log(std::string message){
    std::shared_ptr<LogEntry> logEntry = std::make_shared<LogEntry>();
    logEntry->logType = LogType::LOG;
    GetComposition(logEntry->message);
    logEntry->message += "Log: " + message;
    std::cout << logEntry->message << std::endl;

    RegisterEntry(logEntry);
};

void Logger::LogWarning(std::string message){
    std::shared_ptr<LogEntry> logEntry = std::make_shared<LogEntry>();
    logEntry->logType = LogType::WARNING;
    GetComposition(logEntry->message);
    logEntry->message += "Warning: " + message;
    std::cout << "\e[0;93m" << logEntry->message << "\033[0m" << std::endl;

    RegisterEntry(logEntry);
};

void Logger::LogError(std::string message){
    std::shared_ptr<LogEntry> logEntry = std::make_shared<LogEntry>();
    logEntry->logType = LogType::ERROR;
    GetComposition(logEntry->message);
    logEntry->message += "Error: " + message;
    std::cout << "\x1B[91m" << logEntry->message << "\033[0m" << std::endl;

    RegisterEntry(logEntry);
};

void Logger::RegisterEntry(std::shared_ptr<LogEntry> logEntry){
    logEntry->stackTrace = StackTrace::GetStackTrace();
    LoggerNotifier::newLogEntryHandler->Invoke(logEntry);
    logEntries.push_back(logEntry);
};