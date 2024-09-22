#pragma once

class Time{
    private:
        static float lastUpdate;
    public:
        static float deltaTime;
        static bool PermissionForUpdate();
};