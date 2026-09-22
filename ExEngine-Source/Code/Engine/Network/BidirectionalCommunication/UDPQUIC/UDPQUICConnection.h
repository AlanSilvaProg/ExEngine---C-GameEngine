#pragma once
#include "../IConnectionKind.h"
#include "../../../Logger/Logger.h"
#include <string>
#include <curl/curl.h>

class UDPQUICConnection : public IConnectionKind{
private:
    CURL* handler = nullptr;
    // libcurl's CONNECT_ONLY=2 is documented/supported for WebSocket and HTTP/2 upgrades,
    // but not HTTP/3: for h3 it silently runs the request to completion during Connect()
    // instead of stopping at "connected". So every curl_easy_recv() afterward is guaranteed
    // to fail - this latches after the first failure so UpdateConnection() (polled every
    // frame by NetworkManager::Update) logs it once instead of spamming forever.
    bool receiveUnavailable = false;
public:
    void CreateConnectionHanlder(const std::string socketAddress) override {
        handler = curl_easy_init();
        curl_easy_setopt(handler, CURLOPT_URL, socketAddress.c_str());
        curl_easy_setopt(handler, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_3ONLY);
        curl_easy_setopt(handler, CURLOPT_CONNECT_ONLY, 2L);
        curl_easy_setopt(handler, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(handler, CURLOPT_TIMEOUT, 10L);
    };

    CURLcode Connect() override{
        return curl_easy_perform(handler);
    };

    void UpdateConnection() override {
        if(handler == nullptr || receiveUnavailable) return;

        char buffer[4096];
        size_t bytesReceived = 0;

        CURLcode result = curl_easy_recv(handler, buffer, sizeof(buffer), &bytesReceived);

        if(result == CURLE_AGAIN) return;

        if(result != CURLE_OK){
            receiveUnavailable = true;
            Logger::Log("UDPQUICConnection: recv failed: " + std::string(curl_easy_strerror(result))
                + " (HTTP/3 CONNECT_ONLY already ran the request to completion in libcurl - "
                + "no data is left to receive on this connection)");
            return;
        }

        if(bytesReceived == 0) return;

        Logger::Log("UDPQUICConnection received " + std::to_string(bytesReceived) + " bytes: " + std::string(buffer, bytesReceived));
    };

    void SendMessage(INetworkObject networkObject) override{
        if(handler == nullptr) return;

        const std::string content = networkObject.TestContent();
        size_t bytesSent = 0;

        CURLcode result = curl_easy_send(handler, content.c_str(), content.size(), &bytesSent);

        if(result != CURLE_OK){
            Logger::Log("UDPQUICConnection: send failed: " + std::string(curl_easy_strerror(result)));
            return;
        }

        Logger::Log("UDPQUICConnection sent " + std::to_string(bytesSent) + " bytes: " + content);
    };

    void CleanupHandler() override {
        if(handler == nullptr) return;

        curl_easy_cleanup(handler);
    };
};
