#include "ProcessProgressWindow.h"
#include "../../../../../Engine/Core/Progress/ProcessTracker.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

static const char* ProcessProgressPopupId = "Working##ProcessProgress";
static constexpr float kTwoPi = 6.28318530717958647692f;
static constexpr float kFinishHoldSeconds = 1.0f;

// Fixed content width every row wraps/sizes to. With ImGuiWindowFlags_AlwaysAutoResize, sizing a
// widget off "available width" is circular (the window's width is what we're trying to determine) -
// this constant breaks that cycle so the popup shrink-wraps to exactly its content, no leftover space.
static constexpr float kContentWidth = 320.0f;

// Small "chasing dots" loading indicator - no such widget ships with Dear ImGui, so it's drawn by
// hand: dotCount dots around a circle, each pulsing in alpha offset by its angle so the brightness
// appears to travel around the ring over time.
static void DrawLoadingSpinner(const float radius, const int dotCount, const float speed){
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 topLeft = ImGui::GetCursorScreenPos();
    ImVec2 center(topLeft.x + radius, topLeft.y + radius);
    const float time = static_cast<float>(ImGui::GetTime());
    const float dotRadius = radius * 0.15f;

    for(int i = 0; i < dotCount; i++)
    {
        const float dotAngleOffset = (kTwoPi * i) / dotCount;
        const float angle = dotAngleOffset - time * speed;
        const ImVec2 dotPos(center.x + radius * 0.75f * cosf(angle), center.y + radius * 0.75f * sinf(angle));

        const float pulse = 0.5f * (sinf(time * speed - dotAngleOffset) + 1.0f);
        const float alpha = 0.2f + 0.8f * pulse;

        drawList->AddCircleFilled(dotPos, dotRadius, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, alpha)));
    }

    ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));
};

void ProcessProgressWindow::Draw(int phase){
    if(phase != 1) return;

    const bool hasActiveProcesses = ProcessTracker::HasActiveProcesses();

    if(hasActiveProcesses)
    {
        lastDescription = ProcessTracker::GetCurrentProcess().description;
    }

    // Edge-triggered: only (re)open on the idle -> busy transition, so re-entering this Draw call
    // every frame while busy doesn't fight the popup's own open/close state.
    if(hasActiveProcesses && !wasShowing)
    {
        ImGui::OpenPopup(ProcessProgressPopupId);
        finishing = false; // new work arrived - cancel any hold left over from a previous batch
    }
    else if(!hasActiveProcesses && wasShowing && !finishing)
    {
        finishing = true;
        finishStartTime = static_cast<float>(ImGui::GetTime());
    }
    wasShowing = hasActiveProcesses;

    ImVec2 windowCenter = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(windowCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_AlwaysAutoResize;

    // A modal popup blocks interaction with every other window behind it - that's what gives us
    // "block engine interaction while open" for free, no separate input-disabling needed.
    if(ImGui::BeginPopupModal(ProcessProgressPopupId, nullptr, flags))
    {
        if(finishing && static_cast<float>(ImGui::GetTime()) - finishStartTime >= kFinishHoldSeconds)
        {
            finishing = false;
            ImGui::CloseCurrentPopup();
        }
        else
        {
            const int completed = ProcessTracker::GetCompletedInBatch();
            const int total = ProcessTracker::GetTotalInBatch();

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + kContentWidth);
            ImGui::TextWrapped("%s", lastDescription.empty() ? "Working..." : lastDescription.c_str());
            ImGui::PopTextWrapPos();
            ImGui::Text("Process %d of %d", hasActiveProcesses ? completed + 1 : completed, total);

            ImGui::Spacing();

            // Not every caller reports fine-grained progress (e.g. a script compile is one opaque
            // step) - fall back to how much of the batch is done so the bar isn't stuck at 0% for
            // those, while still respecting a caller's own SetProgress when it is more advanced.
            const float batchFraction = total > 0 ? static_cast<float>(completed) / static_cast<float>(total) : 0.0f;
            const float fraction = finishing ? 1.0f : std::max(ProcessTracker::GetCurrentProcess().progress / 100.0f, batchFraction);

            // Spinner sits beside the bar (to its right) rather than above it, at 25% of its
            // original size (40 -> 10). The bar gets an explicit width so there's room left for it.
            const float spinnerDiameter = 10.0f;
            const float barHeight = ImGui::GetFrameHeight();
            const float lineStartY = ImGui::GetCursorPosY();
            const float barWidth = kContentWidth - spinnerDiameter - ImGui::GetStyle().ItemSpacing.x;

            ImGui::ProgressBar(fraction, ImVec2(barWidth, 0));
            ImGui::SameLine();
            ImGui::SetCursorPosY(lineStartY + (barHeight - spinnerDiameter) * 0.5f);
            DrawLoadingSpinner(spinnerDiameter * 0.5f, 8, 4.0f);
        }

        ImGui::EndPopup();
    }
};
