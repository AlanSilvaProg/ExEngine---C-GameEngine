#pragma once
#include <string>
#include <vector>
#include "ExSerializedField.h"

struct ExSerializedClass{
public:
    std::string className;
    std::vector<ExSerializedField> serializedFields;
};