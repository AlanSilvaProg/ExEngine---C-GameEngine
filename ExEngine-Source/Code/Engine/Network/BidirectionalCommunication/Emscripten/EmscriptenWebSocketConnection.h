#pragma once
#include "../IConnectionKind.h"
#include "../../INetworkObject.h"
#include "../../../Logger/Logger.h"
#include <string>
#include <atomic>
#include <mutex>
#include <deque>
#include <emscripten/websocket.h>

// Browser WebSockets are inherently async (onopen/onmessage/onclose only fire via the JS event
// loop) - blocking here like UDPQUICConnection::Connect() would deadlock the page. Connect() just
// starts the handshake; UpdateConnection() drains callback state, SendMessage() queues until open.
class EmscriptenWebSocketConnection : public IConnectionKind{
private:
    EMSCRIPTEN_WEBSOCKET_T socket = 0;
    std::atomic<bool> opened{false};

    std::mutex receiveMutex;
    std::deque<std::string> receiveQueue;

    std::mutex pendingSendMutex;
    std::deque<std::string> pendingSends;

    static EM_BOOL OnOpen(int eventType, const EmscriptenWebSocketOpenEvent* event, void* userData){
        auto* self = static_cast<EmscriptenWebSocketConnection*>(userData);
        self->opened = true;

        std::lock_guard<std::mutex> lock(self->pendingSendMutex);
        for(const std::string& content : self->pendingSends)
            emscripten_websocket_send_utf8_text(self->socket, content.c_str());

        self->pendingSends.clear();

        return EM_TRUE;
    };

    static EM_BOOL OnError(int eventType, const EmscriptenWebSocketErrorEvent* event, void* userData){
        Logger::LogError("EmscriptenWebSocketConnection: socket error");
        return EM_TRUE;
    };

    static EM_BOOL OnClose(int eventType, const EmscriptenWebSocketCloseEvent* event, void* userData){
        auto* self = static_cast<EmscriptenWebSocketConnection*>(userData);
        self->opened = false;

        Logger::Log("EmscriptenWebSocketConnection: socket closed (code " + std::to_string(event->code) + ")");
        return EM_TRUE;
    };

    static EM_BOOL OnMessage(int eventType, const EmscriptenWebSocketMessageEvent* event, void* userData){
        auto* self = static_cast<EmscriptenWebSocketConnection*>(userData);

        std::lock_guard<std::mutex> lock(self->receiveMutex);
        self->receiveQueue.emplace_back((const char*)event->data, event->numBytes);

        return EM_TRUE;
    };
public:
    void CreateConnectionHanlder(const std::string socketAddress) override {
        if(!emscripten_websocket_is_supported()){
            Logger::LogError("EmscriptenWebSocketConnection: WebSocket is not supported in this browser");
            return;
        }

        EmscriptenWebSocketCreateAttributes attributes;
        emscripten_websocket_init_create_attributes(&attributes);
        attributes.url = socketAddress.c_str();

        socket = emscripten_websocket_new(&attributes);

        if(socket <= 0){
            Logger::LogError("EmscriptenWebSocketConnection: failed to create socket for " + socketAddress);
            return;
        }

        emscripten_websocket_set_onopen_callback(socket, this, OnOpen);
        emscripten_websocket_set_onerror_callback(socket, this, OnError);
        emscripten_websocket_set_onclose_callback(socket, this, OnClose);
        emscripten_websocket_set_onmessage_callback(socket, this, OnMessage);
    };

    bool Connect() override{
        return socket > 0;
    };

    void UpdateConnection() override {
        std::lock_guard<std::mutex> lock(receiveMutex);

        while(!receiveQueue.empty()){
            Logger::Log("EmscriptenWebSocketConnection received " + std::to_string(receiveQueue.front().size()) + " bytes: " + receiveQueue.front());
            receiveQueue.pop_front();
        }
    };

    void SendMessage(INetworkObject networkObject) override{
        if(socket <= 0) return;

        const std::string content = networkObject.TestContent();

        if(!opened){
            std::lock_guard<std::mutex> lock(pendingSendMutex);
            pendingSends.push_back(content);
            return;
        }

        emscripten_websocket_send_utf8_text(socket, content.c_str());
        Logger::Log("EmscriptenWebSocketConnection sent " + std::to_string(content.size()) + " bytes: " + content);
    };

    void CleanupHandler() override {
        if(socket <= 0) return;

        emscripten_websocket_close(socket, 1000, "");
        emscripten_websocket_delete(socket);
        socket = 0;
    };
};
