#pragma once
#include "../../EditorWindow.h"
#include <string>

// Blocking progress overlay: opens automatically whenever ProcessTracker has an active process and
// closes automatically once none remain. Has no close button on purpose - it's entirely reactive to
// ProcessTracker's Begin/End/SetProgress calls, not user interaction.
class ProcessProgressWindow : public EditorWindow{
private:
    bool wasShowing = false;

    // Held at 100% for a beat after the last process ends, instead of vanishing the instant it does.
    bool finishing = false;
    float finishStartTime = 0.0f;

    // ProcessTracker's "current process" goes empty the moment the last one ends, so cache the last
    // description seen while busy to keep showing it during the finishing hold.
    std::string lastDescription;
public:
    ProcessProgressWindow() = default;
    void Draw(const int phase) override; //0 == early 1 == late
};
