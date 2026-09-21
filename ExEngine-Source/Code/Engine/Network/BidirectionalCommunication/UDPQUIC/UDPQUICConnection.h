#pragma once
#include "../IConnectionKind.h"
#include "../../../Logger/Logger.h"
#include <string>
#include <curl/curl.h>

class UDPQUICConnection : public IConnectionKind{
private:
    CURL* handler;
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
        // ToDo: read incoming QUIC data once SocketConnection keeps the handler alive after connect.
    };

    void SendMessage(INetworkObject networkObject) override{
        if(handler == nullptr) return;

        const std::string content = networkObject.TestContent();
        size_t bytesSent = 0;

        // No WebSocket-style framing API exists for QUIC in curl - this writes straight to
        // the raw connect-only socket opened in CreateConnectionHanlder.
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
