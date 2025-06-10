#pragma once
#include <string>
#include <any>

enum ValueType{
    Undefined = 0,
    Bool = 1,
    Int = 2,
    Float = 3,
    String = 4
};

class EComponentSPropertyData{
public:
    std::string name;
    ValueType valueType;
    std::any value;

    EComponentSPropertyData(const std::string name, const std::any value) : name(name), value(value){};
};