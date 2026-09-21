#pragma once
#include "IConnectionKind.h"
#include "TransferType.h"
#include "TCP/TCPWebSocketConnection.h"
#include "UDPQUIC/UDPQUICConnection.h"
#include <memory>

class ConnectionKindFactory{
public:
    inline static std::unique_ptr<IConnectionKind> Create(const TransferType transferType){
        switch(transferType){
            case TransferType::TCP: return std::make_unique<TCPWebSocketConnection>();
            case TransferType::UDPQUIC: return std::make_unique<UDPQUICConnection>();
        }

        return nullptr;
    };
};
