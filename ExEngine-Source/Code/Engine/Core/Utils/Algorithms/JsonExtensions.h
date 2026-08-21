#pragma once
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace JsonExtensions{

    inline nlohmann::json glm_to_json(const glm::vec3& v) {
        return nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }

    inline nlohmann::json glm_to_json(const glm::vec2& v) {
        return nlohmann::json{{"x", v.x}, {"y", v.y}};
    }

    inline void glm_from_json(const nlohmann::json& j, glm::vec3& v) {
        v.x = j.value("x", 0.0f);
        v.y = j.value("y", 0.0f);
        v.z = j.value("z", 0.0f);
    }

    inline void glm_from_json(const nlohmann::json& j, glm::vec2& v) {
        v.x = j.value("x", 0.0f);
        v.y = j.value("y", 0.0f);
    }

};