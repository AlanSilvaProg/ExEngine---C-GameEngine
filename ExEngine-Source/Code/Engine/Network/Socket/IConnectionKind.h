#pragma once
#include <string>
#include <curl/curl.h>

class IConnectionKind{
public:
    virtual CURL* CreateConnectionHanlder(const std::string socketAddress) = 0;
    virtual void CleanupHandler() = 0;
};