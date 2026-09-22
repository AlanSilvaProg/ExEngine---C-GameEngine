#pragma once
#include "../../EditorWindow.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class NetworkTestWindow : public EditorWindow{
private:
    // NetworkRequest test state
    std::string requestUrl = "https://jsonplaceholder.typicode.com/todos/1";
    int requestMethodIndex = 0; // GET, POST
    int requestHttpVersionIndex = 0; // HTTP1_1, HTTP2, HTTP3 (see HttpVersion.h)
    std::atomic<bool> requestInProgress{false};
    std::thread requestThread;
    std::mutex requestResultMutex;
    std::string requestResultContent;
    bool requestHasResult = false;
    bool requestSuccess = false;

    // SocketConnection test state
    std::string socketAddress = "wss://echo.websocket.org/"; // Websocket (TCP) - must be ws:// or wss:// for curl_ws_recv/send to work
    std::string quicAddress = "https://cloudflare-quic.com/"; // UDP QUIC
    int socketTransferTypeIndex = 0; // Bidirectional communication (TCP), UDP QUIC
    std::atomic<bool> socketInProgress{false};
    std::thread socketThread;

    std::atomic<bool> sendInProgress{false};
    std::thread sendThread;

    void DrawNetworkRequestSection();
    void DrawSocketConnectionSection();

    void SendTestRequest();
    void ConnectTestSocket();
    void SendTestMessage();
public:
    ~NetworkTestWindow();

    void Draw(const int phase) override; //0 == early 1 == late
};
