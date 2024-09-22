#pragma once

class RuntimeSettings{
    private:
        static int targetFramesPerSeconds;
        static float timePerFrame;
    public:
        static void SetTargetFps(int targetFps);
        static int GetTargetFps();
        static float GetTimePerFrame();
};