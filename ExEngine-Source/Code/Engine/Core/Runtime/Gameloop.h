#pragma once 

class Gameloop{
    private:
        void Start();
        void Update();
        void FixedUpdate();
        bool CheckFixedUpdatePermission();
    public:
        void InitializeGameloop();
};