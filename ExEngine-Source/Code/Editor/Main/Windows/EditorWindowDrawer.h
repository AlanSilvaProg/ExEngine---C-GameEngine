#pragma once
#include "EditorWindow.h"
#include <vector>
#include <memory>

namespace ExEditor{

class EditorWindowDrawer {
private:
    static std::vector<std::shared_ptr<EditorWindow>> windows;
public:
    EditorWindowDrawer();
    ~EditorWindowDrawer();
    void Draw(const int phase); // 0 == early 1 == late

    static void AddWindow(std::shared_ptr<EditorWindow> window);
    static void RemoveWindow(std::shared_ptr<EditorWindow> window);
};

};