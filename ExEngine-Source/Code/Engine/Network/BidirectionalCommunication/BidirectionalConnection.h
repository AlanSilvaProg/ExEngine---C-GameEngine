#pragma once
#include "IConnectionKind.h"
#include "ConnectionKindFactory.h"
#include "TransferType.h"
#include "../INetworkObject.h"
#include "../../Logger/Logger.h"
#include <atomic>
#include <memory>
#include <string>

struct BidirectionalConnectionData{
private:
    std::string url;
public:
    inline void SetUrl(std::string newUrl){ url = newUrl; };

    inline std::string& GetUrl() { return url; };
};

class BidirectionalConnection{
private:
    std::unique_ptr<IConnectionKind> connectionKind;
    std::atomic<bool> connected{false};
public:
    inline ~BidirectionalConnection(){
        CloseAnyConnection();
    };

    inline void TryConnectTo(const std::string socketAddress, const TransferType transferType){
        CloseAnyConnection();

        connectionKind = ConnectionKindFactory::Create(transferType);
        connectionKind->CreateConnectionHanlder(socketAddress);

        if(!connectionKind->Connect()){
            connectionKind->CleanupHandler();
            connectionKind = nullptr;

            Logger::LogError("Connection Failed");
            return;
        }

        connected = true;

        //ToDo Register all notifications to callback
    };

    inline void UpdateConnection(){
        if(!connected) return;

        connectionKind->UpdateConnection();
    };

    inline void SendMessage(INetworkObject networkObject){
        if(!connected){
            Logger::Log("BidirectionalConnection: cannot send, not connected");
            return;
        }

        connectionKind->SendMessage(networkObject);
    };

    inline void CloseAnyConnection(){
        connected = false;

        if(connectionKind == nullptr) return;

        connectionKind->CleanupHandler();
        connectionKind = nullptr;
    };
};