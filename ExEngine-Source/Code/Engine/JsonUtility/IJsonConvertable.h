#pragma once
#include "nlohmann/json.hpp"

class IJsonConvertable{
    virtual nlohmann::json ToJson() = 0;
    virtual void FromJson(const nlohmann::json& json) = 0;
};