#include "ExSerializedFieldSetter.h"

namespace ExSerializedFieldSetter
{
    bool TrySetValue(const ExSerializedField& field, const std::any& value)
    {
        if (!field.setValue) return false;

        bool applied = field.setValue(value);

        if (applied && field.onFieldChanged)
            field.onFieldChanged();

        return applied;
    }

    bool TrySetValueFromJson(const ExSerializedField& field, const nlohmann::json& value)
    {
        if (!field.setValueFromJson) return false;

        bool applied = field.setValueFromJson(value);

        if (applied && field.onFieldChanged)
            field.onFieldChanged();

        return applied;
    }
}
