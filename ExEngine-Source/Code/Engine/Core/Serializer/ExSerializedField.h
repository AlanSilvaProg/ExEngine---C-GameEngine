#pragma once
#include "string"
#include "functional"

struct ISerializable;

struct ExSerializedField{
    std::string fieldName;
    const std::type_info& fieldType;
    void* field_ptr;
    ISerializable* serializable_ptr;
    bool editable;
    std::function<void()> onFieldChanged = nullptr;
};