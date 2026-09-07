#include "AnimationEditorWindow.h"
#include "../ElementSelectionController.h"
#include "../EntityBrowser/EntityBrowserSelection.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../EditorEvents/EditorCommandEventHandler.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

AnimationEditorWindow::AnimationEditorWindow(){
    *EditorCommandEventHandler::deleteCmmd += [this](){ this->DeleteSelectedKeyframe(); };
};

void AnimationEditorWindow::Draw(const int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::animationEditorEnabled){
        if(currentEntity != nullptr){
            //Reset to the original value if appliable
            ResetEntityState();
            currentEntity = nullptr;
            isPlaying = false;
            SetSelectedKeyframeTime(-1.0f);
            isDraggingKeyframe = false;
            //Cache current entity state if appliable
        }
        return;
    }

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

    if(currentEntity != entity)
    {
        //Reset to the original value if appliable
        ResetEntityState();
        currentEntity = entity;
        isPlaying = false;
        SetSelectedKeyframeTime(-1.0f);
        isDraggingKeyframe = false;
        //Cache current entity state if appliable
        CacheEntityState();
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
            DrawTimeline(entity, animationComponent);
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
        AddKeyframe(entity);
    }

    if(ImGui::Button("Save Changes"))
    {
        SaveChanges(entity);
    }
};

void AnimationEditorWindow::AddKeyframe(const std::shared_ptr<EntityCS> entity){
    auto animationComponent = entity->GetComponent<AnimationComponent>();

    if(animationComponent == nullptr) return;

    auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    const auto& componentsPool = ecsManager->GetEntityComponentPools();
    const auto entityId = entity->GetId();

    EntityContent stepContent;

    for(const auto& pool : componentsPool)
    {
        auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pool);
        if(castedPoolManager == nullptr) continue;

        auto component = castedPoolManager->GetComponent(entityId);
        if(component == nullptr) continue;

        // The AnimationComponent drives the timeline rather than being an animated target;
        // capturing it here would nest the whole step history inside every new keyframe.
        if(component->GetComponentId() == AnimationComponent::ComponentId) continue;

        const auto componentId = component->GetComponentId();

        // ToJson() must be bound to a named variable before calling .items() on it - items()
        // returns a proxy holding a reference back into the json object, which would otherwise
        // dangle the moment the temporary from ToJson() is destroyed at the end of this statement.
        const auto componentJson = component->ToJson();
        for(const auto& [fieldName, value] : componentJson.items())
        {
            stepContent.componentUpdates.emplace_back(componentId, fieldName, value);
        }
    }

    AnimationStep step;
    step.secondsToTrigger = animationComponent->currentTime;
    step.ResetStep();
    step.SetNewSetStateContent(stepContent);

    auto& steps = animationComponent->animationInfo.animationSteps;
    auto insertPos = std::lower_bound(steps.begin(), steps.end(), step.secondsToTrigger,
        [](const AnimationStep& existing, float time){ return existing.secondsToTrigger < time; });

    if(insertPos != steps.end() && insertPos->secondsToTrigger == step.secondsToTrigger)
        *insertPos = step;
    else
        steps.insert(insertPos, step);
};

void AnimationEditorWindow::SaveChanges(const std::shared_ptr<EntityCS> entity){
    auto animationComponent = entity->GetComponent<AnimationComponent>();
    if(animationComponent == nullptr) return;

    // Update just the AnimationComponent's cached entry so ResetEntityState (which runs when
    // switching entities or closing the window) stops undoing keyframe edits made here, while
    // any unsaved edits to the entity's other components are still reverted as before.
    for(auto& entry : currentEntityOriginalState)
    {
        const int id = entry["id"];
        if(id == animationComponent->GetComponentId())
        {
            entry["data"] = animationComponent->ToJson();
            break;
        }
    }
};

void AnimationEditorWindow::SetSelectedKeyframeTime(const float time){
    selectedKeyframeTime = time;

    // Claim the shared Delete command while a keyframe is selected, so it targets the
    // keyframe instead of the entity that's still the main editor selection.
    ElementSelectionController::SetDeleteCommandOverridden(time >= 0.0f);
};

void AnimationEditorWindow::DeleteSelectedKeyframe(){
    if(selectedKeyframeTime < 0.0f) return;
    if(currentEntity == nullptr) return;

    auto animationComponent = currentEntity->GetComponent<AnimationComponent>();
    if(animationComponent == nullptr) return;

    auto& steps = animationComponent->animationInfo.animationSteps;
    const auto stepToRemove = std::find_if(steps.begin(), steps.end(), [this](const AnimationStep& step){
        return std::abs(step.secondsToTrigger - selectedKeyframeTime) < 0.0001f;
    });

    if(stepToRemove != steps.end()) steps.erase(stepToRemove);

    SetSelectedKeyframeTime(-1.0f);
};

void AnimationEditorWindow::CacheEntityState(){
    if(currentEntity == nullptr) return;

    auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    const auto& componentsPool = ecsManager->GetEntityComponentPools();
    const auto entityId = currentEntity->GetId();

    currentEntityOriginalState = nlohmann::json::array();

    for(const auto& pool : componentsPool)
    {
        auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(pool);
        if(castedPoolManager == nullptr) continue;

        auto component = castedPoolManager->GetComponent(entityId);
        if(component == nullptr) continue;

        currentEntityOriginalState.push_back({
            {"id", component->GetComponentId()},
            {"data", component->ToJson()}
        });
    }
}

void AnimationEditorWindow::ResetEntityState(){
    if(currentEntity == nullptr) return;

    auto ecsManager = EditorInterfaceGetters::engine->GetECSManagerPtr();
    const auto& componentsPool = ecsManager->GetEntityComponentPools();
    const auto entityId = currentEntity->GetId();

    for(const auto& entry : currentEntityOriginalState)
    {
        const int id = entry["id"];
        if(id < 0 || id >= static_cast<int>(componentsPool.size())) continue;

        auto castedPoolManager = std::dynamic_pointer_cast<EComponentSPoolManager>(componentsPool[id]);
        if(castedPoolManager == nullptr) continue;

        auto component = castedPoolManager->GetComponent(entityId);
        if(component == nullptr) continue; // component removed since caching

        component->FromJson(entry["data"]);
    }

    currentEntityOriginalState = nlohmann::json::array();
}

void AnimationEditorWindow::ApplyEvaluatedState(const std::shared_ptr<EntityCS> entity, AnimationStep* currentAnimationState){
    if(currentAnimationState == nullptr || currentAnimationState->WasReturned()) return;

    for(const auto& componentUpdate : currentAnimationState->GetStepStateContent().componentUpdates)
    {
        entity->ApplyComponentUpdate(componentUpdate);
    }
};

void AnimationEditorWindow::DrawTimeline(const std::shared_ptr<EntityCS> entity, const std::shared_ptr<AnimationComponent> animationComponent){
    constexpr float kTimelineMaxSeconds = 1000.0f;
    constexpr float kFineTickThresholdSeconds = 20.0f;

    auto getCurrentTime = [&]() -> float {
        return animationComponent != nullptr ? animationComponent->currentTime : previewPlayheadTime;
    };
    auto setCurrentTime = [&](float time){
        time = std::clamp(time, 0.0f, kTimelineMaxSeconds);
        if(animationComponent != nullptr)
        {
            const auto currentAnimDuration = animationComponent->animationInfo.GetAnimationDurationInSecs();
            if(currentAnimDuration > 0){
                const auto currentAnimationState = animationComponent->EvaluateTo(time > currentAnimDuration ? currentAnimDuration : time);

                ApplyEvaluatedState(entity, currentAnimationState); 
            }

            animationComponent->currentTime = time;
        }
        else
        {
            previewPlayheadTime = time;
        }
    };

    const float animationDuration = animationComponent != nullptr ? animationComponent->animationInfo.GetAnimationDurationInSecs() : 0.0f;
    const bool canPlay = animationComponent != nullptr && animationDuration > 0.0f;

    // Playback is driven here rather than through AnimationSystem/AnimationManager, since no
    // system currently ticks the animation queues while just editing (no game running).
    if(isPlaying)
    {
        if(!canPlay)
        {
            isPlaying = false;
        }
        else
        {
            const float nextTime = animationComponent->currentTime + ImGui::GetIO().DeltaTime;
            isPlaying = nextTime < animationDuration;

            const auto currentAnimationState = animationComponent->EvaluateTo(nextTime > animationDuration ? animationDuration : nextTime);
            ApplyEvaluatedState(entity, currentAnimationState);
        }
    }

    float headTime = getCurrentTime();
    ImGui::SetNextItemWidth(100);
    bool headTimeEdited = false;
    if(ImGui::DragFloat("Head Time", &headTime, 0.05f, 0.0f, kTimelineMaxSeconds, "%.2fs"))
    {
        setCurrentTime(headTime);
        headTimeEdited = true;
    }

    ImGui::SameLine();
    const float transportButtonSize = ImGui::GetFrameHeight();

    ImGui::BeginDisabled(!canPlay || isPlaying);
    ImTextureID playTextureId = (ImTextureID)(intptr_t)EditorInterfaceGetters::defaultIconsInformation["PlayIcon"]->GetTexture();
    if(ImGui::ImageButton("AnimationPlayButton", playTextureId, ImVec2(transportButtonSize, transportButtonSize)))
        isPlaying = true;
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!canPlay);
    if(ImGui::Button("Restart"))
    {
        animationComponent->EvaluateTo(0.0f);
        isPlaying = true;
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!isPlaying);
    ImTextureID pauseTextureId = (ImTextureID)(intptr_t)EditorInterfaceGetters::defaultIconsInformation["PauseIcon"]->GetTexture();
    if(ImGui::ImageButton("AnimationPauseButton", pauseTextureId, ImVec2(transportButtonSize, transportButtonSize)))
        isPlaying = false;
    ImGui::EndDisabled();

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
                // Only test for a keyframe hit on the click's first frame, so a click that
                // misses every keyframe still scrubs the playhead for the rest of the drag.
                if(ImGui::IsItemActivated() && animationComponent != nullptr)
                {
                    constexpr float kKeyframeHitRadius = 7.0f;
                    isDraggingKeyframe = false;

                    for(const auto& step : animationComponent->animationInfo.animationSteps)
                    {
                        const float stepX = areaMin.x + step.secondsToTrigger * timelinePixelsPerSecond;
                        if(std::abs(io.MousePos.x - stepX) <= kKeyframeHitRadius)
                        {
                            SetSelectedKeyframeTime(step.secondsToTrigger);
                            setCurrentTime(step.secondsToTrigger);
                            isDraggingKeyframe = true;
                            draggingKeyframeTime = step.secondsToTrigger;
                            break;
                        }
                    }

                    if(!isDraggingKeyframe) SetSelectedKeyframeTime(-1.0f);
                }

                if(isDraggingKeyframe && animationComponent != nullptr)
                {
                    auto& steps = animationComponent->animationInfo.animationSteps;
                    const auto draggedStep = std::find_if(steps.begin(), steps.end(), [this](const AnimationStep& step){
                        return std::abs(step.secondsToTrigger - draggingKeyframeTime) < 0.0001f;
                    });

                    if(draggedStep != steps.end())
                    {
                        float newTime = std::clamp((io.MousePos.x - areaMin.x) / timelinePixelsPerSecond, 0.0f, kTimelineMaxSeconds);

                        // Nudge away from any other keyframe so the drag never collapses two
                        // keyframes onto the same time, which would make them ambiguous to
                        // select/delete afterwards.
                        constexpr float kMinKeyframeGap = 0.001f;
                        for(const auto& other : steps)
                        {
                            if(&other == &(*draggedStep)) continue;
                            if(std::abs(other.secondsToTrigger - newTime) < kMinKeyframeGap)
                                newTime = newTime < other.secondsToTrigger ? other.secondsToTrigger - kMinKeyframeGap : other.secondsToTrigger + kMinKeyframeGap;
                        }
                        newTime = std::clamp(newTime, 0.0f, kTimelineMaxSeconds);

                        draggedStep->secondsToTrigger = newTime;
                        draggingKeyframeTime = newTime;
                        SetSelectedKeyframeTime(newTime);
                        setCurrentTime(newTime);

                        std::stable_sort(steps.begin(), steps.end(), [](const AnimationStep& a, const AnimationStep& b){
                            return a.secondsToTrigger < b.secondsToTrigger;
                        });
                    }
                }
                else if(!isDraggingKeyframe)
                {
                    const float newTime = (io.MousePos.x - areaMin.x) / timelinePixelsPerSecond;
                    setCurrentTime(newTime);
                }
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

        if(animationComponent != nullptr)
        {
            constexpr float kKeyframeHalfSize = 6.0f;
            const float keyframeY = areaMin.y + 30.0f;

            for(const auto& step : animationComponent->animationInfo.animationSteps)
            {
                const float keyframeX = areaMin.x + step.secondsToTrigger * timelinePixelsPerSecond;
                const bool isSelected = std::abs(step.secondsToTrigger - selectedKeyframeTime) < 0.0001f;
                const ImU32 color = isSelected ? IM_COL32(70, 140, 255, 255) : IM_COL32(230, 230, 230, 255);

                drawList->AddQuadFilled(
                    ImVec2(keyframeX, keyframeY - kKeyframeHalfSize),
                    ImVec2(keyframeX + kKeyframeHalfSize, keyframeY),
                    ImVec2(keyframeX, keyframeY + kKeyframeHalfSize),
                    ImVec2(keyframeX - kKeyframeHalfSize, keyframeY),
                    color
                );
            }
        }

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