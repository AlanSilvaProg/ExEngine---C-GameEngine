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
    std::atomic<bool> requestInProgress{false};
    std::thread requestThread;
    std::mutex requestResultMutex;
    std::string requestResultContent;
    bool requestHasResult = false;
    bool requestSuccess = false;

    // SocketConnection test state
    std::string socketAddress = "https://echo.websocket.org/.sse";
    int socketTransferTypeIndex = 0; // TCP, UDP
    std::atomic<bool> socketInProgress{false};
    std::thread socketThread;

    void DrawNetworkRequestSection();
    void DrawSocketConnectionSection();

    void SendTestRequest();
    void ConnectTestSocket();
public:
    ~NetworkTestWindow();

    void Draw(const int phase) override; //0 == early 1 == late
};
