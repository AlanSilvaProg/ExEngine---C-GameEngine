#pragma once
#include <string>

class RuntimeSettings{
    private:
        static int targetFramesPerSeconds;
        static float timePerFrame;
        static std::string externalTextEditorPath;
        static bool physicsEnabled;
        static bool drawAllPhysics;
    public:
        static void SetTargetFps(int targetFps);
        static int GetTargetFps();
        static float GetTimePerFrame();

        //empty means the machine's default program for the file type should be used
        static void SetExternalTextEditorPath(const std::string& path);
        static const std::string& GetExternalTextEditorPath();

        static void SetPhysicsEnabled(bool enabled);
        static bool GetPhysicsEnabled();

        static void SetDrawAllPhysics(bool enabled);
        static bool GetDrawAllPhysics();
};