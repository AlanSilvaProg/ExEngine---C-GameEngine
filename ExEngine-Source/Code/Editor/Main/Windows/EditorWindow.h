#pragma once

class EditorWindow{
public: 
    void virtual Draw(int phase) = 0; // 0 == early 1 == late
};