#pragma once 

class Gameloop{
    private:
        bool isRunning;
        float lastUpdate;

        void FixedUpdate();
        void Stop();

        void ProcessInputPhase();
        void ProcessCollisionPhase();
    public:
        void Initialize();
        void ExecuteGameLoop();
};