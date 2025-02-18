#pragma once
#include "../../EventSystem/EventNotifier.h"
#include <memory>

class AppEventsHandler : public EventNotifier{
public:
    static std::unique_ptr<AppEventsHandler> onApplicationQuitHandler;
    //ToDo: focus handler
    //ToDo: server handler

    static void Create();
};