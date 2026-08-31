#include "AnimationEditorWindow.h"
#include "../ElementSelectionController.h"
#include "../EntityBrowser/EntityBrowserSelection.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

void AnimationEditorWindow::Draw(const int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::animationEditorEnabled) return;

    std::shared_ptr<EntityCS> entity = nullptr;
    std::shared_ptr<AnimationComponent> animationComponent = nullptr;

    auto selectedElement = ElementSelectionController::GetCurrentSelection();
    if(selectedElement != nullptr && selectedElement->GetType() == EditorSelectableType::Entity)
    {
        auto entityBrowserSelection = dynamic_cast<EntityBrowserSelection*>(selectedElement);
        auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
        entity = ecsManager->GetEntity(entityBrowserSelection->GetSelectedEntityId());

        if(entity != nullptr && ecsManager->HasComponent<AnimationComponent>(entity))
            animationComponent = entity->GetComponent<AnimationComponent>();
        else
            entity = nullptr;
    }

    ImGui::SetNextWindowSizeConstraints(ImVec2(600, 300), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

    if(ImGui::Begin("Animation", &EditorInterfaceGetters::animationEditorEnabled, ImGuiWindowFlags_NoDocking)) // 0
    {
        auto availableSpace = ImGui::GetContentRegionAvail();

        if(ImGui::BeginChild("Animation Left", {availableSpace.x * 0.2f, availableSpace.y}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX)) // 1
        {
            DrawEntityInfo(entity);
        }
        ImGui::EndChild(); // 1

        ImGui::SameLine();

        if(ImGui::BeginChild("Animation Right", {0, 0}, ImGuiChildFlags_Borders)) // 2
        {
            DrawTimeline(animationComponent);
        }
        ImGui::EndChild(); // 2
    }
    ImGui::End(); // 0
};

void AnimationEditorWindow::DrawEntityInfo(const std::shared_ptr<EntityCS> entity){
    if(entity == nullptr)
    {
        ImGui::TextDisabled("No entity selected");
        return;
    }

    ImGui::TextWrapped("%s", entity->GetName().c_str());
};

void AnimationEditorWindow::DrawTimeline(const std::shared_ptr<AnimationComponent> animationComponent){
    // The ruler always spans the same fixed range regardless of whether an animation is
    // bound, so the timeline is buildable/navigable even with nothing selected.
    constexpr float kTimelineMaxSeconds = 1000.0f;
    // Below this many visible seconds we switch from whole-second ticks to a
    // clock-like subdivision: a labeled half-second mark plus 0.1s dashes.
    constexpr float kFineTickThresholdSeconds = 20.0f;

    ImGui::SetNextItemWidth(200);
    ImGui::SliderFloat("Zoom", &timelinePixelsPerSecond, 20.0f, 1000.0f, "%.0f px/s");

    if(ImGui::BeginChild("Timeline Scroll Area", {0, 0}, ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar))
    {
        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float availableHeight = ImGui::GetContentRegionAvail().y;
        const float timelineWidth = kTimelineMaxSeconds * timelinePixelsPerSecond;

        ImGui::InvisibleButton("Timeline Area", ImVec2(timelineWidth, availableHeight));

        const bool isActive = ImGui::IsItemActive();
        const ImVec2 areaMin = ImGui::GetItemRectMin();
        const ImVec2 areaMax = ImGui::GetItemRectMax();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(areaMin, areaMax, IM_COL32(40, 40, 40, 255));

        // Only build ticks for the currently scrolled-into-view range. Native child
        // scrolling already refuses to go past t=0 or past the fixed max duration.
        const float scrollX = ImGui::GetScrollX();
        const float visibleStart = std::max(scrollX / timelinePixelsPerSecond, 0.0f);
        const float visibleEnd = std::min((scrollX + availableWidth) / timelinePixelsPerSecond, kTimelineMaxSeconds);

        const bool fineTicks = (visibleEnd - visibleStart) < kFineTickThresholdSeconds;
        const float minorStep = fineTicks ? 0.1f : 1.0f;
        const int ticksPerSecond = fineTicks ? 10 : 1;

        const int firstTick = std::max(static_cast<int>(std::floor(visibleStart / minorStep)) - 1, 0);
        const int lastTick = static_cast<int>(std::ceil(visibleEnd / minorStep)) + 1;

        for(int i = firstTick; i <= lastTick; ++i)
        {
            const float t = i * minorStep;
            if(t > kTimelineMaxSeconds) break;

            const bool isWholeSecond = (i % ticksPerSecond) == 0;
            const bool isHalfSecond = fineTicks && !isWholeSecond && (i % ticksPerSecond) == ticksPerSecond / 2;

            const float x = areaMin.x + t * timelinePixelsPerSecond;
            const float tickTop = isWholeSecond ? areaMin.y : (isHalfSecond
                ? areaMin.y + (areaMax.y - areaMin.y) * 0.35f
                : areaMin.y + (areaMax.y - areaMin.y) * 0.6f);
            const int tickAlpha = isWholeSecond ? 255 : (isHalfSecond ? 200 : 130);

            drawList->AddLine(ImVec2(x, tickTop), ImVec2(x, areaMax.y), IM_COL32(90, 90, 90, tickAlpha));

            if(isWholeSecond)
                drawList->AddText(ImVec2(x + 2, areaMin.y + 2), IM_COL32(200, 200, 200, 255), (std::to_string(static_cast<int>(std::lround(t))) + "s").c_str());
            else if(isHalfSecond)
            {
                char label[16];
                std::snprintf(label, sizeof(label), "%.1f", t);
                drawList->AddText(ImVec2(x + 2, areaMin.y + 2), IM_COL32(160, 160, 160, 255), label);
            }
        }

        if(isActive && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            const float mouseX = ImGui::GetIO().MousePos.x;
            float newTime = (mouseX - areaMin.x) / timelinePixelsPerSecond;
            newTime = std::clamp(newTime, 0.0f, kTimelineMaxSeconds);

            if(animationComponent != nullptr)
            {
                const float clipDuration = std::max(animationComponent->animationInfo.GetAnimationDurationInSecs(), 0.0f);
                animationComponent->EvaluateTo(std::min(newTime, clipDuration));
            }
            else
            {
                previewPlayheadTime = newTime;
            }
        }

        const float playheadTime = animationComponent != nullptr ? animationComponent->currentTime : previewPlayheadTime;
        const float playheadX = areaMin.x + std::clamp(playheadTime, 0.0f, kTimelineMaxSeconds) * timelinePixelsPerSecond;
        drawList->AddLine(ImVec2(playheadX, areaMin.y), ImVec2(playheadX, areaMax.y), IM_COL32(255, 60, 60, 255), 2.0f);
        drawList->AddTriangleFilled(
            ImVec2(playheadX - 5, areaMin.y),
            ImVec2(playheadX + 5, areaMin.y),
            ImVec2(playheadX, areaMin.y + 8),
            IM_COL32(255, 60, 60, 255)
        );
    }
    ImGui::EndChild();
};
