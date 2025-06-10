#pragma once
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>

class FileManagement{
private:
    static bool LoadFileAsJson(std::string key, nlohmann::json& result);
    static bool SaveFileAtPath(std::filesystem::path path, std::string& value);
public:
    static bool SaveFile(std::filesystem::path path, std::string& value);
    static bool SaveFile(std::string key, std::string& value);
    static bool LoadFile(std::string key, std::string& result);

    template<typename T>
    static bool SaveFile(std::filesystem::path path, T value);
    template<typename T>
    static bool SaveFile(std::string key, T value);
    template<typename T>
    static bool LoadFromJson(std::string key, T& result);
};

template<typename T>
bool FileManagement::LoadFromJson(std::string key, T& result){
    nlohmann::json json;
    if(!LoadFileAsJson(key, json))
        return false;

    json.get_to(result);

    return true;
};

template<typename T>
bool FileManagement::SaveFile(std::filesystem::path key, T value){
    nlohmann::json json = nlohmann::json::parse(value);
    
    return SaveFile(key, json.template get<std::string>());
};

template<typename T>
bool FileManagement::SaveFile(std::string key, T value){
    nlohmann::json json = nlohmann::json::parse(value);
    
    return SaveFile(key, json.template get<std::string>());
};