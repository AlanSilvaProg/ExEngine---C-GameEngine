#pragma once
#include "IConnectionKind.h"
#include "TransferType.h"
#include "TCP/TCPConnection.h"
#include "UDP/UDPConnection.h"
#include <memory>

class ConnectionKindFactory{
public:
    inline static std::unique_ptr<IConnectionKind> Create(const TransferType transferType){
        switch(transferType){
            case TransferType::TCP: return std::make_unique<TCPConnection>();
            case TransferType::UDP: return std::make_unique<UDPConnection>();
        }

        return nullptr;
    };
};
