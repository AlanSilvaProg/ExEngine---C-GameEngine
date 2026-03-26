#pragma once
#include "LogEntry.h"
#include "../Core/EventSystem/Event.h"
#include <memory>

class LoggerNotifier : public Event<std::shared_ptr<LogEntry>&>{
public:
    static std::unique_ptr<LoggerNotifier> newLogEntryHandler;
};

inline std::unique_ptr<LoggerNotifier> LoggerNotifier::newLogEntryHandler = std::make_unique<LoggerNotifier>();