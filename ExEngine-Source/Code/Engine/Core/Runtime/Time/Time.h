#pragma once

class Time{
    private:
        static float lastUpdate;
        static float lastFixedUpdate;
    public:
        static float deltaTime;
        static float fixedDeltaTime;
        static bool PermissionForUpdate();
};