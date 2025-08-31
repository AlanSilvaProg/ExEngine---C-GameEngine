#pragma once

struct ExSerializedClass;

struct ISerializable{
public:
    virtual ExSerializedClass Serialize() = 0;
};