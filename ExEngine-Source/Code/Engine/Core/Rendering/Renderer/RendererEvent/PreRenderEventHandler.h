#pragma once
#include <memory>
#include "../../../EventSystem/Event.h"

class PreRenderEventHandler : public Event<int>{
public:
    static std::unique_ptr<PreRenderEventHandler> handler;

    static void Create();
};