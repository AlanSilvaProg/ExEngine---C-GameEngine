#pragma once
#include "../IConnectionKind.h"
#include <string>
#include <curl/curl.h>

class TCPConnection : public IConnectionKind{
private: 
    CURL* handler;
public: 
    CURL* CreateConnectionHanlder(const std::string socketAddress) override {
        handler = curl_easy_init();
        curl_easy_setopt(handler, CURLOPT_URL, socketAddress);
        curl_easy_setopt(handler, CURLOPT_CONNECT_ONLY, 0L);
        return handler;
    };

    void CleanupHandler() override {
        if(handler == nullptr) return;
        
        curl_easy_cleanup(handler);
    };
};