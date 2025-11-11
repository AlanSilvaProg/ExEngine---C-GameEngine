#pragma once
#include "../../EditorWindow.h"

class AssetBrowserWindow : public EditorWindow{
private:
    bool cancelInstantly;
    bool firstUpdate = true;
    float targetPosition;
    float currentPosition;
    float windowVelocity = 1000;

    void UpdatePositionTarget(float& targetPosition, float& currentPosition, int& h);
public:
    AssetBrowserWindow() = default;
    void Draw(int phase) override; //0 == early 1 == late
};