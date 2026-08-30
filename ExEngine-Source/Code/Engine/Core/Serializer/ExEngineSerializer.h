#pragma once
#include "string"
#include "vector"
#include "any"
#include "functional"
#include "Demangle.h"
#include "ISerializable.h"
#include "ExSerializedField.h"
#include "../../JsonUtility/IJsonConvertable.h"
#include "../Utils/Algorithms/JsonExtensions.h"
#include <glm/glm.hpp>

#ifndef EX_SERIALIZER
#define EX_SERIALIZER(obj, fieldName, editable) \
ExSerializedField{ #fieldName, typeid(decltype(obj.fieldName)), &obj.fieldName, GetSerializablePtr(&obj.fieldName), editable, nullptr, MakeFieldSetter(&obj.fieldName), MakeFieldJsonSetter(&obj.fieldName) }
#endif

#ifndef EX_SERIALIZER_CB
#define EX_SERIALIZER_CB(obj, fieldName, editable, callback) \
ExSerializedField{ #fieldName, typeid(decltype(obj.fieldName)), &obj.fieldName, GetSerializablePtr(&obj.fieldName), editable, callback, MakeFieldSetter(&obj.fieldName), MakeFieldJsonSetter(&obj.fieldName) }
#endif

#ifndef EX_SERIALIZE_CLASS
#define EX_SERIALIZE_CLASS(...) \
virtual ExSerializedClass Serialize() override{ \
    return ExSerializedClass{ \
        Demangle(typeid(*this).name()), \
        { __VA_ARGS__ } \
    }; \
};
#endif

template<typename T>
ISerializable* GetSerializablePtr(T* ptr) {
    if constexpr (std::is_base_of<ISerializable, T>::value) {
        return ptr;
    } else {
        return nullptr;
    }
}

// Generates a type-erased setter bound to T at compile time, so applying a value
// never needs to compare fieldType against a list of known types at the call site.
template<typename T>
std::function<bool(const std::any&)> MakeFieldSetter(T* ptr) {
    return [ptr](const std::any& value) -> bool {
        if (const T* casted = std::any_cast<T>(&value)) {
            *ptr = *casted;
            return true;
        }
        return false;
    };
}

// Same idea, but committing an already-parsed json payload straight into the field —
// used when the new value comes from disk/network (e.g. ComponentUpdate) instead of
// from a live C++ value, so it never needs a std::any round trip nor a runtime type switch.
template<typename T>
std::function<bool(const nlohmann::json&)> MakeFieldJsonSetter(T* ptr) {
    return [ptr](const nlohmann::json& json) -> bool {
        if constexpr (std::is_base_of<IJsonConvertable, T>::value) {
            ptr->FromJson(json);
            return true;
        }
        else if constexpr (std::is_same<T, glm::vec2>::value || std::is_same<T, glm::vec3>::value) {
            JsonExtensions::glm_from_json(json, *ptr);
            return true;
        }
        else {
            try {
                *ptr = json.get<T>();
                return true;
            } catch (const nlohmann::json::exception&) {
                return false;
            }
        }
    };
}