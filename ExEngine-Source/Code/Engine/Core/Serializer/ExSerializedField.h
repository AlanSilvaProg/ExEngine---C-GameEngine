#pragma once
#include "string"

struct ISerializable;

struct ExSerializedField{
    std::string fieldName;
    const std::type_info& fieldType;
    void* field_ptr;
    ISerializable* serializable_ptr;
    bool editable;
};