#include "NoCameraEventHandler.h"

std::unique_ptr<NoCameraEventHandler> NoCameraEventHandler::noCameraHandler = nullptr;

void NoCameraEventHandler::Create(){
    if(noCameraHandler == nullptr)
    {
        noCameraHandler = std::make_unique<NoCameraEventHandler>();
    }
};
