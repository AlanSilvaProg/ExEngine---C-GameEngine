#include "NetworkManager.h"

std::unique_ptr<NetworkRequest> NetworkManager::networkRequestPtr;
std::unique_ptr<BidirectionalConnection> NetworkManager::bidirectionalConnectionPtr;

int NetworkManager::Create(){
    networkRequestPtr = std::make_unique<NetworkRequest>();
    bidirectionalConnectionPtr = std::make_unique<BidirectionalConnection>();
    return 1;
};

void NetworkManager::Update(){
    bidirectionalConnectionPtr->UpdateConnection();
};