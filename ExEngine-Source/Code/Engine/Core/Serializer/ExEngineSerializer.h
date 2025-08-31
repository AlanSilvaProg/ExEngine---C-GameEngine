#pragma once 
#include "string"
#include "vector"
#include "Demangle.h"
#include "ISerializable.h"
#include "ExSerializedField.h"

#ifndef EX_SERIALIZER
#define EX_SERIALIZER(obj, fieldName, editable) \
ExSerializedField{ #fieldName, typeid(decltype(obj.fieldName)), &obj.fieldName, GetSerializablePtr(&obj.fieldName), editable }
#endif

template<typename T>
ISerializable* GetSerializablePtr(T* ptr) {
    if constexpr (std::is_base_of<ISerializable, T>::value) {
        return ptr;
    } else {
        return nullptr; 
    }
}