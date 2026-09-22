#pragma once
#include "Url/NetworkRequest.h"
#include "BidirectionalCommunication/BidirectionalConnection.h"
#include <memory>

class NetworkManager{
public:
    static std::unique_ptr<NetworkRequest> networkRequestPtr;
    static std::unique_ptr<BidirectionalConnection> bidirectionalConnectionPtr;

    static int Create();
    static void Update();
};

inline static int AutoCreation = NetworkManager::Create();