#pragma once
#include "../INetworkObject.h"
#include <string>
#include <curl/curl.h>

class IConnectionKind{
public:
    virtual void CreateConnectionHanlder(const std::string socketAddress) = 0;
    virtual CURLcode Connect() = 0;
    virtual void UpdateConnection() = 0;
    virtual void SendMessage(INetworkObject networkObject) = 0;
    virtual void CleanupHandler() = 0;
};