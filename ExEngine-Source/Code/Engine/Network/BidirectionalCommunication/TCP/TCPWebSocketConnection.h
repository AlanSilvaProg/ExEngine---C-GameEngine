#pragma once
#include "../IConnectionKind.h"
#include "../../../Core/EventSystem/EventNotifier.h"
#include "../../../Logger/Logger.h"
#include <string>
#include <curl/curl.h>

class TCPWebSocketConnection : public IConnectionKind{
private: 
    CURL* handler;
public: 
    void CreateConnectionHanlder(const std::string socketAddress) override {
        handler = curl_easy_init();
        curl_easy_setopt(handler, CURLOPT_URL, socketAddress.c_str());
        curl_easy_setopt(handler, CURLOPT_CONNECT_ONLY, 2L);
        curl_easy_setopt(handler, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(handler, CURLOPT_TIMEOUT, 10L);
    };

    CURLcode Connect() override{
        return curl_easy_perform(handler);
    };

    void UpdateConnection() override {
        if(handler == nullptr) return;

        char buffer[4096];
        size_t bytesReceived = 0;
        const curl_ws_frame* frame = nullptr;

        CURLcode result = curl_ws_recv(handler, buffer, sizeof(buffer), &bytesReceived, &frame);

        if(result == CURLE_AGAIN) return;

        if(result != CURLE_OK){
            Logger::Log("TCPWebSocketConnection: recv failed: " + std::string(curl_easy_strerror(result)));
            return;
        }

        if(bytesReceived == 0) return;

        if(frame != nullptr && (frame->flags & CURLWS_CLOSE)){
            Logger::Log("TCPWebSocketConnection: received close frame");
            return;
        }

        Logger::Log("TCPWebSocketConnection received " + std::to_string(bytesReceived) + " bytes: " + std::string(buffer, bytesReceived));
    };

    void SendMessage(INetworkObject networkObject) override{
        if(handler == nullptr) return;

        const std::string content = networkObject.TestContent();
        size_t bytesSent = 0;

        CURLcode result = curl_ws_send(handler, content.c_str(), content.size(), &bytesSent, 0, CURLWS_TEXT);

        if(result != CURLE_OK){
            Logger::Log("TCPWebSocketConnection: send failed: " + std::string(curl_easy_strerror(result)));
            return;
        }

        Logger::Log("TCPWebSocketConnection sent " + std::to_string(bytesSent) + " bytes: " + content);
    };

    void CleanupHandler() override {
        if(handler == nullptr) return;
        
        curl_easy_cleanup(handler);
    };
};