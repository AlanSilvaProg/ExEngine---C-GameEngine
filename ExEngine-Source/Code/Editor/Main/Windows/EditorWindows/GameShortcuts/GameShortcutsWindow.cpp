#include "GameShortcutsWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Core/Runtime/App.h"
#include "../../../../../Engine/Core/Runtime/Settings/RuntimeSettings.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"
#include <imgui.h>

static const char* PlayConfirmationPopupId = "Start Play?##GameShortcutsPlayConfirmation";

// Fixed content width the popup wraps/sizes to. With ImGuiWindowFlags_AlwaysAutoResize, sizing
// off "available width" is circular (the window's width is what we're trying to determine) -
// this constant breaks that cycle so the popup shrink-wraps to a comfortably readable width.
static constexpr float kPlayConfirmationContentWidth = 280.0f;

void GameShortcutsWindow::Draw(const int phase){
    if(phase != 1) return;

    // Auto-collapse into the thin bar the moment Play starts, regardless of which view is active.
    if(App::isPlaying && !wasPlaying)
        collapsed = true;
    wasPlaying = App::isPlaying;

    if(EditorInterfaceGetters::viewMode != EditorViewMode::GameView) return;

    // Force a fully opaque background regardless of the current style's window alpha.
    ImVec4 opaqueBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    opaqueBg.w = 1.0f;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, opaqueBg);

    // Glued to the horizontal center, right below the main menu bar - ImGui shrinks the main
    // viewport's work area by the menu bar's height, so WorkPos.y already points just past it.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float centerX = viewport->WorkPos.x + viewport->WorkSize.x * 0.5f;
    ImGui::SetNextWindowPos(ImVec2(centerX, viewport->WorkPos.y), ImGuiCond_Always, ImVec2(0.5f, 0.0f));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar;

    if(!ImGui::Begin("Game Shortcuts", nullptr, windowFlags))
    {
        ImGui::End();
        ImGui::PopStyleColor();
        return;
    }

    const ImVec2 buttonSize(32, 32);

    if(!collapsed)
    {
        const float rowStartX = ImGui::GetCursorScreenPos().x;

        // ImageButton adds the style's FramePadding around its image size, while Button uses the
        // given size as-is - match that here so the Stop button renders at the exact same footprint
        // as the Play/Pause ImageButtons instead of looking smaller.
        const ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        const ImVec2 stopButtonSize(buttonSize.x + framePadding.x * 2.0f, buttonSize.y + framePadding.y * 2.0f);

        if(App::isPlaying)
        {
            // No dedicated "stop" asset exists yet, so the icon is drawn as a plain square glyph
            // on top of a regular button to keep the same look/feel as the Play/Pause icon buttons.
            const bool stopPressed = ImGui::Button("##Stop", stopButtonSize);
            const ImVec2 min = ImGui::GetItemRectMin();
            const ImVec2 max = ImGui::GetItemRectMax();
            const float padding = stopButtonSize.x * 0.28f;
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(min.x + padding, min.y + padding),
                ImVec2(max.x - padding, max.y - padding),
                ImGui::GetColorU32(ImGuiCol_Text)
            );

            if(stopPressed) Stop();
        }
        else
        {
            ImTextureID playTextureId = (ImTextureID)(intptr_t)EditorInterfaceGetters::defaultIconsInformation["PlayIcon"]->GetTexture();
            if(ImGui::ImageButton("PlayButton", playTextureId, buttonSize))
                Play();
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(!App::isPlaying);
        ImTextureID pauseTextureId = (ImTextureID)(intptr_t)EditorInterfaceGetters::defaultIconsInformation["PauseIcon"]->GetTexture();
        if(ImGui::ImageButton("PauseButton", pauseTextureId, buttonSize))
            Pause();
        ImGui::EndDisabled();

        lastKnownRowWidth = ImGui::GetItemRectMax().x - rowStartX;

        ImGui::Spacing();
    }

    DrawCollapseToggleBar(lastKnownRowWidth);

    ImGui::End();
    ImGui::PopStyleColor();

    DrawPlayConfirmationPopup();
};

void GameShortcutsWindow::DrawPlayConfirmationPopup(){
    if(playConfirmation && !ImGui::IsPopupOpen(PlayConfirmationPopupId))
        ImGui::OpenPopup(PlayConfirmationPopupId);

    ImVec2 windowCenter = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(windowCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

    // A modal popup blocks interaction with every other window behind it, same as the
    // hot reload progress popup - no separate input-disabling needed.
    if(ImGui::BeginPopupModal(PlayConfirmationPopupId, nullptr, flags))
    {
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + kPlayConfirmationContentWidth);
        ImGui::TextWrapped("Starting Play will save the world in its current state and begin. Continue?");
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if(ImGui::Button("Play"))
        {
            StartPlay();
            playConfirmation = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel"))
        {
            playConfirmation = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
};

void GameShortcutsWindow::DrawCollapseToggleBar(float width){
    const float barHeight = 6.0f;

    ImGui::InvisibleButton("##GameShortcutsCollapseToggle", ImVec2(width, barHeight));
    if(ImGui::IsItemClicked())
        collapsed = !collapsed;

    const ImVec2 barMin = ImGui::GetItemRectMin();
    const ImVec2 barMax = ImGui::GetItemRectMax();
    const ImU32 barColor = ImGui::IsItemHovered() ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Button);
    ImGui::GetWindowDrawList()->AddRectFilled(barMin, barMax, barColor, barHeight * 0.5f);
};

void GameShortcutsWindow::StartPlay(){
    EditorInterfaceGetters::Save();
    App::isPlaying = true;
};

void GameShortcutsWindow::Play(){
    if(RuntimeSettings::GetAutoSaveOnPlay())
    {
        StartPlay();
        return;
    }

    playConfirmation = true;
};

void GameShortcutsWindow::Stop(){
    App::isPlaying = false;
    ECSWorldManager::ReloadCurrentWorldState();
};

void GameShortcutsWindow::Pause(){
};
