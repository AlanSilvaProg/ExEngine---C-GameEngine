#pragma once
#include "IConnectionKind.h"
#include "ConnectionKindFactory.h"
#include "TransferType.h"
#include "../../Logger/Logger.h"
#include <curl/curl.h>
#include <memory>
#include <string>

struct SocketConnectionData{
private:
    std::string url;
public:
    inline void SetUrl(std::string newUrl){ url = newUrl; };

    inline std::string& GetUrl() { return url; };
};

class SocketConnection{
private:
    std::unique_ptr<IConnectionKind> connectionKind;
public:
    void TryConnectTo(const std::string socketAddress, const TransferType transferType){
        connectionKind = ConnectionKindFactory::Create(transferType);
        auto handler = connectionKind->CreateConnectionHanlder(socketAddress);

        auto result = curl_easy_perform(handler);

        connectionKind->CleanupHandler();
        connectionKind = nullptr;

        if(result != CURLE_OK){
            Logger::Log("DEU RUIM: " + std::string(curl_easy_strerror(result)));
            return;
        }

        Logger::Log("Deu BOM");
    };
};