#pragma once
#include "../INetworkObject.h"
#include <string>

class IConnectionKind{
public:
    virtual void CreateConnectionHanlder(const std::string socketAddress) = 0;
    virtual bool Connect() = 0;
    virtual void UpdateConnection() = 0;
    virtual void SendMessage(INetworkObject networkObject) = 0;
    virtual void CleanupHandler() = 0;
};
