#pragma once
#include "any"
#include "ExSerializedField.h"

// Single place that knows how to commit a new value into an ExSerializedField.
// Both the editor (inspector) and the engine (component update application) go through here
// so the set of supported field types never has to be duplicated across layers.
namespace ExSerializedFieldSetter
{
    bool TrySetValue(const ExSerializedField& field, const std::any& value);
    bool TrySetValueFromJson(const ExSerializedField& field, const nlohmann::json& value);
}
