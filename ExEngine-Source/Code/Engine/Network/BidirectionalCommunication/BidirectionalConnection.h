#pragma once
#include "IConnectionKind.h"
#include "ConnectionKindFactory.h"
#include "TransferType.h"
#include "../INetworkObject.h"
#include "../../Logger/Logger.h"
#include <curl/curl.h>
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
public:
    inline ~BidirectionalConnection(){
        CloseAnyConnection();
    };

    inline void TryConnectTo(const std::string socketAddress, const TransferType transferType){
        CloseAnyConnection();

        connectionKind = ConnectionKindFactory::Create(transferType);
        connectionKind->CreateConnectionHanlder(socketAddress);

        auto result = connectionKind->Connect();

        if(result != CURLE_OK){
            connectionKind->CleanupHandler();
            connectionKind = nullptr;

            Logger::Log("DEU RUIM: " + std::string(curl_easy_strerror(result)));
            return;
        }

        //ToDo Register all notifications to callback
    };

    inline void UpdateConnection(){
        if(connectionKind == nullptr) return;

        connectionKind->UpdateConnection();
    };

    inline void SendMessage(INetworkObject networkObject){
        if(connectionKind == nullptr){
            Logger::Log("BidirectionalConnection: cannot send, not connected");
            return;
        }

        connectionKind->SendMessage(networkObject);
    };

    inline void CloseAnyConnection(){
        if(connectionKind == nullptr) return;

        connectionKind->CleanupHandler();
        connectionKind = nullptr;
    };
};