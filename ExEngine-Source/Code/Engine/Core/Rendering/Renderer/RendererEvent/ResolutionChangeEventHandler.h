#pragma once
#include <memory>
#include "../../../EventSystem/Event.h"

class ResolutionChangeEventHandler : public Event<int, int>{
public:
    static std::unique_ptr<ResolutionChangeEventHandler> resolutionChangeHandler;

    static void Create();
    static void NotifyResolutionChange(int width, int height);
};