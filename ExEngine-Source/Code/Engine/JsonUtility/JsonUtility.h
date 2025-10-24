#pragma once
#include "nlohmann/json.hpp"

class JsonUtility{
public:
    template<typename T>
    inline static nlohmann::json ToJson(T& obj){
        return obj.ToJson();
    };

    template<typename T>
    inline static void FromJson(T& obj, nlohmann::json& json){
        obj.FromJson(json);
    };
};