#pragma once
#include <memory>
#include "../EventSystem/EventNotifier.h"

class NoCameraEventHandler : public EventNotifier{
public:
    static std::unique_ptr<NoCameraEventHandler> noCameraHandler;

    static void Create();
};
