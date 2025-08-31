#pragma once
#include "../Section.h"

class WindowSection : public Section {
public:
    WindowSection() = default;
    ~WindowSection() = default;

    void virtual Draw() override;
};