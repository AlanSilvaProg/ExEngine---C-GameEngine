#pragma once
#include "../../EditorWindow.h"

class GameShortcutsWindow : public EditorWindow{
private:
    bool playConfirmation = false;
    bool collapsed = false;
    bool wasPlaying = false;
    float lastKnownRowWidth = 64.0f;

    void Play();
    void Stop();
    void Pause();
    void StartPlay();
    void DrawPlayConfirmationPopup();
    void DrawCollapseToggleBar(float width);
public:
    void Draw(const int phase) override; //0 == early 1 == late
};
