#pragma once 

class EditorInterface{
private:
    void InitializeEditor();
    void EarlyUpdate();
    void PreRender();
public:
    EditorInterface();
    ~EditorInterface();
};