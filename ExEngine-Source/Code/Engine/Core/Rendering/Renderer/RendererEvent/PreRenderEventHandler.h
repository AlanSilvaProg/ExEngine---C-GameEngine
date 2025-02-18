#pragma once
#include <memory>
#include "../../../EventSystem/EventNotifier.h"

class PreRenderEventHandler : public EventNotifier{
public:
    static std::unique_ptr<PreRenderEventHandler> preRenderHandler;
    static std::unique_ptr<PreRenderEventHandler> postRenderHandler;
    static std::unique_ptr<PreRenderEventHandler> postRenderPresentHandler;

    static void Create();
};