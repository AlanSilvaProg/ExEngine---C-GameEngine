#pragma once
#include "../Section.h"

class FeaturesSection : public Section {
public:
    FeaturesSection() = default;
    ~FeaturesSection() = default;

    void virtual Draw() override;
};
