#pragma once
#include <exception>
#include <string>

enum class FileWatcherError {
    InvalidPath,
    PermissionDenied,
    ResourceExhausted,
    PlatformApiFailure,
    ThreadingError
};

class FileWatcherException : public std::exception {
private:
    FileWatcherError m_errorType;
    std::string m_message;
    
public:
    FileWatcherException(FileWatcherError error, const std::string& message)
        : m_errorType(error), m_message(message) {}
    
    const char* what() const noexcept override {
        return m_message.c_str();
    }
    
    FileWatcherError GetErrorType() const {
        return m_errorType;
    }
};