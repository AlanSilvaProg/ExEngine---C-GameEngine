#include "Logger.h"
#include <iostream>
#include <stdlib.h> 
#include <chrono>
#include <ctime>

std::vector<LogEntry> Logger::logEntries;
const std::string Logger::logPrefix = "[ExEngine]-";

std::string Logger::GetCurrentTime(){
    std::string outPut(30, '\0');
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(&outPut[0], outPut.size(), "%d-%b-%Y %H:%M:%S", localtime(&now));

    return "[" + outPut + "]";
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
    LogEntry logEntry;
    logEntry.logType = LogType::LOG;
    GetComposition(logEntry.message);
    logEntry.message += "Log: " + message;
    std::cout << logEntry.message << std::endl;

    logEntries.push_back(logEntry);
};

void Logger::LogWarning(std::string message){
    LogEntry logEntry;
    logEntry.logType = LogType::WARNING;
    GetComposition(logEntry.message);
    logEntry.message += "Warning: " + message;
    std::cout << "\e[0;93m" << logEntry.message << "\033[0m" << std::endl;

    logEntries.push_back(logEntry);
};

void Logger::LogError(std::string message){
    LogEntry logEntry;
    logEntry.logType = LogType::ERROR;
    GetComposition(logEntry.message);
    logEntry.message += "Error: " + message;
    std::cout << "\x1B[91m" << logEntry.message << "\033[0m" << std::endl;

    logEntries.push_back(logEntry);
};