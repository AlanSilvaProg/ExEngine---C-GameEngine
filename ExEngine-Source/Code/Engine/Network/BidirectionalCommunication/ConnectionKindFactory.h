#pragma once
#include "IConnectionKind.h"
#include "TransferType.h"
#include <memory>

// TCPWebSocketConnection (libcurl) and UDPQUICConnection (msquic) talk to raw native sockets,
// which the browser sandbox does not expose - an Emscripten build swaps both out for
// EmscriptenWebSocketConnection, backed by the browser's own WebSocket object instead.
#ifdef __EMSCRIPTEN__
#include "Emscripten/EmscriptenWebSocketConnection.h"
#else
#include "TCP/TCPWebSocketConnection.h"
#include "UDPQUIC/UDPQUICConnection.h"
#endif

class ConnectionKindFactory{
public:
    inline static std::unique_ptr<IConnectionKind> Create(const TransferType transferType){
        switch(transferType){
#ifdef __EMSCRIPTEN__
            case TransferType::EMSCRIPTENWEBSOCKET: return std::make_unique<EmscriptenWebSocketConnection>();
#else
            case TransferType::TCP: return std::make_unique<TCPWebSocketConnection>();
            case TransferType::UDPQUIC: return std::make_unique<UDPQUICConnection>();
#endif
        }

        return nullptr;
    };
};
