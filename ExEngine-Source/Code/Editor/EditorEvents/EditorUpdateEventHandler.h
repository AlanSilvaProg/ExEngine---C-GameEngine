#pragma once
#include "../../Engine/Core/EventSystem/EventNotifier.h"
#include <memory>

class EditorUpdateEventHandler : public EventNotifier {
public:
    static std::unique_ptr<EditorUpdateEventHandler> earlyHandler;
    static std::unique_ptr<EditorUpdateEventHandler> lateHandler;
    static std::unique_ptr<EditorUpdateEventHandler> postRenderPresentHandler;

    static void Create();
};