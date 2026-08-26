#pragma once
#include "../../EditorWindow.h"

class GameShortcutsWindow : public EditorWindow{
private:
    bool playConfirmation = false;

    void Play();
    void Stop();
    void Pause();
    void StartPlay();
    void DrawPlayConfirmationPopup();
public:
    void Draw(int phase) override; //0 == early 1 == late
};
