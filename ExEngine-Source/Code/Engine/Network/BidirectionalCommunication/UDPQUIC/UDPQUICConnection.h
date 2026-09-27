#pragma once
#include "../IConnectionKind.h"
#include "../../../Logger/Logger.h"
#include <string>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <msquic.h>

// Raw QUIC transport via msquic - not an HTTP/3 client, both ends must share the ALPN below.
// Replaces a previous libcurl CONNECT_ONLY approach, which ran HTTP/3 requests to completion
// during Connect() instead of stopping at "connected", leaving nothing to send/receive after.
class UDPQUICConnection : public IConnectionKind{
private:
    inline static const char* Alpn = "exengine";

    inline static std::mutex globalMutex;
    inline static const QUIC_API_TABLE* api = nullptr;
    inline static HQUIC registration = nullptr;
    inline static HQUIC configuration = nullptr;
    inline static int refCount = 0;

    HQUIC connection = nullptr;
    HQUIC stream = nullptr;
    std::string host;
    uint16_t port = 0;
    bool acquiredGlobalState = false;

    std::mutex connectMutex;
    std::condition_variable connectCv;
    bool connectDone = false;
    bool connectSucceeded = false;

    // ConnectionShutdown()/StreamShutdown() only request an async teardown - the handles stay
    // alive, and callbacks may still land on a MsQuic worker thread, until each one's
    // SHUTDOWN_COMPLETE event fires. CleanupHandler() blocks on these so `this` is never
    // destroyed (via the owning unique_ptr) while such a callback could still touch it.
    std::mutex shutdownMutex;
    std::condition_variable shutdownCv;
    bool streamClosed = true;
    bool connectionClosed = true;

    std::mutex receiveMutex;
    std::string receiveBuffer;

    static bool AcquireGlobalState(){
        std::lock_guard<std::mutex> lock(globalMutex);

        if(refCount > 0){
            refCount++;
            return true;
        }

        if(QUIC_FAILED(MsQuicOpen2(&api))){
            Logger::LogError("UDPQUICConnection: failed to open the MsQuic API");
            api = nullptr;
            return false;
        }

        QUIC_REGISTRATION_CONFIG registrationConfig{ "ExEngine", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
        if(QUIC_FAILED(api->RegistrationOpen(&registrationConfig, &registration))){
            Logger::LogError("UDPQUICConnection: failed to open the MsQuic registration");
            MsQuicClose(api);
            api = nullptr;
            return false;
        }

        QUIC_SETTINGS settings{};
        settings.IsSet.IdleTimeoutMs = TRUE;
        settings.IdleTimeoutMs = 10000;

        QUIC_BUFFER alpnBuffer{ (uint32_t)strlen(Alpn), (uint8_t*)Alpn };

        if(QUIC_FAILED(api->ConfigurationOpen(registration, &alpnBuffer, 1, &settings, sizeof(settings), nullptr, &configuration))){
            Logger::LogError("UDPQUICConnection: failed to open the MsQuic configuration");
            api->RegistrationClose(registration);
            registration = nullptr;
            MsQuicClose(api);
            api = nullptr;
            return false;
        }

        QUIC_CREDENTIAL_CONFIG credentialConfig{};
        credentialConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
        // No trusted CA is configured for this client yet, so certificate validation is
        // skipped. ToDo revisit once servers we connect to have known/pinned certificates.
        credentialConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

        if(QUIC_FAILED(api->ConfigurationLoadCredential(configuration, &credentialConfig))){
            Logger::LogError("UDPQUICConnection: failed to load MsQuic client credentials");
            api->ConfigurationClose(configuration);
            configuration = nullptr;
            api->RegistrationClose(registration);
            registration = nullptr;
            MsQuicClose(api);
            api = nullptr;
            return false;
        }

        refCount = 1;
        return true;
    };

    static void ReleaseGlobalState(){
        std::lock_guard<std::mutex> lock(globalMutex);

        if(api == nullptr || --refCount > 0) return;

        api->ConfigurationClose(configuration);
        configuration = nullptr;
        api->RegistrationClose(registration);
        registration = nullptr;
        MsQuicClose(api);
        api = nullptr;
    };

    // Accepts "host:port", or the same wrapped in a "scheme://host:port/path" URL, defaulting
    // to port 443 when none is given.
    static bool ParseAddress(const std::string& socketAddress, std::string& outHost, uint16_t& outPort){
        std::string value = socketAddress;

        const size_t schemeSeparator = value.find("://");
        if(schemeSeparator != std::string::npos) value = value.substr(schemeSeparator + 3);

        const size_t pathSeparator = value.find('/');
        if(pathSeparator != std::string::npos) value = value.substr(0, pathSeparator);

        const size_t portSeparator = value.rfind(':');
        if(portSeparator != std::string::npos){
            outHost = value.substr(0, portSeparator);
            outPort = (uint16_t)std::stoi(value.substr(portSeparator + 1));
        } else {
            outHost = value;
            outPort = 443;
        }

        return !outHost.empty();
    };

    static QUIC_STATUS QUIC_API StreamCallback(HQUIC streamHandle, void* context, QUIC_STREAM_EVENT* event){
        auto* self = static_cast<UDPQUICConnection*>(context);

        switch(event->Type){
            case QUIC_STREAM_EVENT_RECEIVE: {
                std::lock_guard<std::mutex> lock(self->receiveMutex);
                for(uint32_t i = 0; i < event->RECEIVE.BufferCount; i++){
                    const QUIC_BUFFER& buffer = event->RECEIVE.Buffers[i];
                    self->receiveBuffer.append((const char*)buffer.Buffer, buffer.Length);
                }
                break;
            }
            case QUIC_STREAM_EVENT_SEND_COMPLETE:
                delete[] (uint8_t*)event->SEND_COMPLETE.ClientContext;
                break;
            case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
                Logger::Log("UDPQUICConnection: peer shut down its send direction");
                break;
            case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
                api->StreamClose(streamHandle);
                {
                    std::lock_guard<std::mutex> lock(self->shutdownMutex);
                    if(self->stream == streamHandle) self->stream = nullptr;
                    self->streamClosed = true;
                }
                self->shutdownCv.notify_all();
                break;
            default: break;
        }

        return QUIC_STATUS_SUCCESS;
    };

    static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC connectionHandle, void* context, QUIC_CONNECTION_EVENT* event){
        auto* self = static_cast<UDPQUICConnection*>(context);

        switch(event->Type){
            case QUIC_CONNECTION_EVENT_CONNECTED: {
                std::lock_guard<std::mutex> lock(self->connectMutex);
                self->connectDone = true;
                self->connectSucceeded = true;
                self->connectCv.notify_all();
                break;
            }
            case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
            case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER: {
                std::lock_guard<std::mutex> lock(self->connectMutex);
                self->connectDone = true;
                self->connectSucceeded = false;
                self->connectCv.notify_all();
                break;
            }
            case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
                api->ConnectionClose(connectionHandle);
                {
                    std::lock_guard<std::mutex> lock(self->shutdownMutex);
                    if(self->connection == connectionHandle) self->connection = nullptr;
                    self->connectionClosed = true;
                }
                self->shutdownCv.notify_all();
                break;
            default: break;
        }

        return QUIC_STATUS_SUCCESS;
    };
public:
    void CreateConnectionHanlder(const std::string socketAddress) override {
        if(!ParseAddress(socketAddress, host, port)){
            Logger::LogError("UDPQUICConnection: invalid address: " + socketAddress);
        }
    };

    bool Connect() override{
        if(host.empty()) return false;
        if(!AcquireGlobalState()) return false;
        acquiredGlobalState = true;

        if(QUIC_FAILED(api->ConnectionOpen(registration, ConnectionCallback, this, &connection))){
            Logger::LogError("UDPQUICConnection: failed to open connection");
            return false;
        }
        connectionClosed = false;

        std::unique_lock<std::mutex> lock(connectMutex);
        connectDone = false;
        connectSucceeded = false;

        if(QUIC_FAILED(api->ConnectionStart(connection, configuration, QUIC_ADDRESS_FAMILY_UNSPEC, host.c_str(), port))){
            Logger::LogError("UDPQUICConnection: failed to start connection to " + host + ":" + std::to_string(port));
            return false;
        }

        const bool signaled = connectCv.wait_for(lock, std::chrono::seconds(10), [this]{ return connectDone; });
        lock.unlock();

        if(!signaled){
            Logger::LogError("UDPQUICConnection: connection to " + host + ":" + std::to_string(port) + " timed out");
            return false;
        }

        if(!connectSucceeded){
            Logger::LogError("UDPQUICConnection: connection to " + host + ":" + std::to_string(port) + " failed");
            return false;
        }

        if(QUIC_FAILED(api->StreamOpen(connection, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, this, &stream))){
            Logger::LogError("UDPQUICConnection: failed to open stream");
            return false;
        }
        streamClosed = false;

        if(QUIC_FAILED(api->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE))){
            Logger::LogError("UDPQUICConnection: failed to start stream");
            return false;
        }

        return true;
    };

    void UpdateConnection() override {
        if(connection == nullptr) return;

        std::lock_guard<std::mutex> lock(receiveMutex);
        if(receiveBuffer.empty()) return;

        Logger::Log("UDPQUICConnection received " + std::to_string(receiveBuffer.size()) + " bytes: " + receiveBuffer);
        receiveBuffer.clear();
    };

    void SendMessage(INetworkObject networkObject) override{
        if(stream == nullptr) return;

        const std::string content = networkObject.TestContent();

        // QUIC_BUFFER and its backing bytes are allocated together and freed once
        // QUIC_STREAM_EVENT_SEND_COMPLETE hands the buffer back via ClientSendContext.
        auto* raw = new uint8_t[sizeof(QUIC_BUFFER) + content.size()];
        auto* quicBuffer = (QUIC_BUFFER*)raw;
        quicBuffer->Buffer = raw + sizeof(QUIC_BUFFER);
        quicBuffer->Length = (uint32_t)content.size();
        memcpy(quicBuffer->Buffer, content.data(), content.size());

        if(QUIC_FAILED(api->StreamSend(stream, quicBuffer, 1, QUIC_SEND_FLAG_NONE, raw))){
            Logger::LogError("UDPQUICConnection: send failed");
            delete[] raw;
            return;
        }

        Logger::Log("UDPQUICConnection sent " + std::to_string(content.size()) + " bytes: " + content);
    };

    void CleanupHandler() override {
        if(stream != nullptr) api->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL, 0);
        if(connection != nullptr) api->ConnectionShutdown(connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);

        {
            std::unique_lock<std::mutex> lock(shutdownMutex);
            shutdownCv.wait_for(lock, std::chrono::seconds(5), [this]{ return streamClosed && connectionClosed; });
        }

        if(acquiredGlobalState){
            ReleaseGlobalState();
            acquiredGlobalState = false;
        }
    };
};
