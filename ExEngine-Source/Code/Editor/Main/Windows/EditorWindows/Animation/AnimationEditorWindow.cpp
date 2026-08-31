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

    if(ImGui::Button("Add Keyframe"))
    {
        AddKeyframe();
    }

    if(ImGui::Button("Save Data"))
    {
        SaveData();
    }
};

void AnimationEditorWindow::AddKeyframe(){
    // create a new keyframe ( animationStep ) with the current entity state
};

void AnimationEditorWindow::SaveData(){
    //save this animations data into the component
};

void AnimationEditorWindow::ResetEntityState(){
    // Bring back the entity to its natural state, without animation
}

void AnimationEditorWindow::DrawTimeline(const std::shared_ptr<AnimationComponent> animationComponent){
    constexpr float kTimelineMaxSeconds = 1000.0f;
    constexpr float kFineTickThresholdSeconds = 20.0f;

    auto getCurrentTime = [&]() -> float {
        return animationComponent != nullptr ? animationComponent->currentTime : previewPlayheadTime;
    };
    auto setCurrentTime = [&](float time){
        time = std::clamp(time, 0.0f, kTimelineMaxSeconds);
        if(animationComponent != nullptr)
        {
            const float clipDuration = std::max(animationComponent->animationInfo.GetAnimationDurationInSecs(), 0.0f);
            animationComponent->EvaluateTo(std::min(time, clipDuration));
        }
        else
        {
            previewPlayheadTime = time;
        }
    };

    float headTime = getCurrentTime();
    ImGui::SetNextItemWidth(100);
    bool headTimeEdited = false;
    if(ImGui::DragFloat("Head Time", &headTime, 0.05f, 0.0f, kTimelineMaxSeconds, "%.2fs"))
    {
        setCurrentTime(headTime);
        headTimeEdited = true;
    }

    if(ImGui::BeginChild("Timeline Scroll Area", {0, 0}, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        const ImVec2 viewportPos = ImGui::GetWindowPos();
        const ImVec2 viewportSize = ImGui::GetWindowSize();

        // Same thickness as a native scrollbar, so it respects the engine's style config.
        const float zoomBarHeight = ImGui::GetStyle().ScrollbarSize;
        constexpr float kZoomBarSpacing = 4.0f;

        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const float canvasHeight = ImGui::GetContentRegionAvail().y - zoomBarHeight - kZoomBarSpacing;
        const float timelineWidth = kTimelineMaxSeconds * timelinePixelsPerSecond;

        ImGui::InvisibleButton("Timeline Area", ImVec2(timelineWidth, canvasHeight));

        const bool isActive = ImGui::IsItemActive();
        const ImVec2 areaMin = ImGui::GetItemRectMin();
        const ImVec2 areaMax = ImGui::GetItemRectMax();

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(areaMin, areaMax, IM_COL32(40, 40, 40, 255));

        // Left/Right arrow keys nudge the view by 1 second while hovering the timeline.
        if(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            if(ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                ImGui::SetScrollX(std::max(ImGui::GetScrollX() - timelinePixelsPerSecond, 0.0f));
            if(ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                ImGui::SetScrollX(std::min(ImGui::GetScrollX() + timelinePixelsPerSecond, ImGui::GetScrollMaxX()));
        }

        if(headTimeEdited)
        {
            const float editedPlayheadX = getCurrentTime() * timelinePixelsPerSecond;
            float desiredScroll = ImGui::GetScrollX();
            if(editedPlayheadX < desiredScroll)
                desiredScroll = editedPlayheadX;
            else if(editedPlayheadX > desiredScroll + availableWidth)
                desiredScroll = editedPlayheadX - availableWidth;

            ImGui::SetScrollX(std::clamp(desiredScroll, 0.0f, ImGui::GetScrollMaxX()));
        }

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

            if(isWholeSecond)
            {
                // Only the whole-second marks cross the full height of the timeline.
                drawList->AddLine(ImVec2(x, areaMin.y), ImVec2(x, areaMax.y), IM_COL32(90, 90, 90, 255));
                drawList->AddText(ImVec2(x + 2, areaMin.y + 2), IM_COL32(200, 200, 200, 255), (std::to_string(static_cast<int>(std::lround(t))) + "s").c_str());
                continue;
            }

            // Sub-second dashes stay pinned to the top and stay small; their label sits
            // right below where the dash ends instead of floating up at the top.
            const float tickHeight = isHalfSecond ? 12.0f : 6.0f;
            const float tickBottom = areaMin.y + tickHeight;
            const int tickAlpha = isHalfSecond ? 200 : 130;

            drawList->AddLine(ImVec2(x, areaMin.y), ImVec2(x, tickBottom), IM_COL32(90, 90, 90, tickAlpha));

            if(isHalfSecond)
            {
                char label[16];
                std::snprintf(label, sizeof(label), "%.1f", t);
                const ImVec2 labelSize = ImGui::CalcTextSize(label);
                drawList->AddText(ImVec2(x - labelSize.x * 0.5f, tickBottom + 2), IM_COL32(160, 160, 160, 255), label);
            }
        }

        if(isActive && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            ImGuiIO& io = ImGui::GetIO();
            const bool isCtrlOrCmdPressed = io.KeyCtrl || io.KeySuper;

            if(isCtrlOrCmdPressed)
            {
                // Same modifier+drag the editor camera uses to pan, applied here to scroll the timeline.
                ImGui::SetScrollX(std::clamp(ImGui::GetScrollX() - io.MouseDelta.x, 0.0f, ImGui::GetScrollMaxX()));
            }
            else
            {
                const float newTime = (io.MousePos.x - areaMin.x) / timelinePixelsPerSecond;
                setCurrentTime(newTime);
            }
        }

        const float playheadTime = getCurrentTime();
        const float playheadX = areaMin.x + std::clamp(playheadTime, 0.0f, kTimelineMaxSeconds) * timelinePixelsPerSecond;
        drawList->AddLine(ImVec2(playheadX, areaMin.y), ImVec2(playheadX, areaMax.y), IM_COL32(255, 60, 60, 255), 2.0f);
        drawList->AddTriangleFilled(
            ImVec2(playheadX - 5, areaMin.y),
            ImVec2(playheadX + 5, areaMin.y),
            ImVec2(playheadX, areaMin.y + 8),
            IM_COL32(255, 60, 60, 255)
        );

        // Thin full-width bar in place of the old scrollbar: dragging it sets zoom, not scroll.
        // Borrows the scrollbar's own colors/rounding/thickness so it reads as a native scrollbar.
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::SetCursorScreenPos(ImVec2(viewportPos.x, viewportPos.y + viewportSize.y - zoomBarHeight));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_ScrollbarBg]);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_ScrollbarBg]);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.Colors[ImGuiCol_ScrollbarBg]);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, style.Colors[ImGuiCol_ScrollbarGrab]);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, style.Colors[ImGuiCol_ScrollbarGrabActive]);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, style.GrabMinSize);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.ScrollbarRounding);
        ImGui::SetNextItemWidth(viewportSize.x);
        ImGui::SliderFloat("##timelineZoom", &timelinePixelsPerSecond, 20.0f, 1000.0f, "");
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(5);
    }
    ImGui::EndChild();
};
