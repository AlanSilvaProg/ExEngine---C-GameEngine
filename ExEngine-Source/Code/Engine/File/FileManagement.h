#pragma once
#include "nlohmann/json.hpp"
#include <string>
#include <fstream>
#include "../JsonUtility/JsonUtility.h"

class FileManagement{
private:
    //Save & Load
    static bool SaveFileAtPath(std::filesystem::path path, std::string& value);
    static bool SaveFileAtPath(std::filesystem::path path, const char* value);
    static bool LoadFileAsJson(std::string key, nlohmann::json& result);
    static bool LoadFileAsJson(std::filesystem::path path, nlohmann::json& result);

    //Creation
    static bool CreateFileAtPath(std::filesystem::path path, std::string& value);
    static bool CreateFileAtPath(std::filesystem::path path, const char* value);

    static void ValidateExtension(std::filesystem::path& path);
public:
    //Save & Load
    static bool SaveFile(std::filesystem::path path, std::string value);
    static bool SaveFile(std::string key, std::string value);
    static bool SaveFile(std::filesystem::path path, const char* value);
    static bool SaveFile(std::string key, const char* value);
    static bool LoadFile(std::string key, std::string& result);
    static bool LoadFile(std::filesystem::path key, std::string& result);

    //Creation
    static bool CreateFile(std::filesystem::path path, std::string value);
    static bool CreateFile(std::string key, std::string value);
    static bool CreateFile(std::filesystem::path path, const char* value);
    static bool CreateFile(std::string key, const char* value);

    static bool CreateDirectory(std::string path);
    static bool CreateDirectory(std::filesystem::path path);

    //Save & Load
    template<typename T>
    static bool SaveFile(std::filesystem::path path, T& value);
    template<typename T>
    static bool SaveFile(std::string key, T& value);
    template<typename T>
    static bool LoadFromJson(std::string key, T& result);
    template<typename T>
    static bool LoadFromJson(std::filesystem::path path, T& result);

    //Creation
    template<typename T>
    static bool CreateFile(std::filesystem::path path, T& value);
    template<typename T>
    static bool CreateFile(std::string key, T& value);
};

//Save & Load
template<typename T>
bool FileManagement::LoadFromJson(std::string key, T& result){
    nlohmann::json json;
    if(!LoadFileAsJson(key, json))
        return false;

    JsonUtility::FromJson(result, json);

    return true;
};

template<typename T>
bool FileManagement::LoadFromJson(std::filesystem::path path, T& result){
    nlohmann::json json;
    if(!LoadFileAsJson(path, json))
        return false;

    JsonUtility::FromJson(result, json);

    return true;
};

template<typename T>
bool FileManagement::SaveFile(std::filesystem::path key, T& value){
    nlohmann::json json = JsonUtility::ToJson(value);
    
    return SaveFile(key, json.template get<std::string>());
};

template<typename T>
bool FileManagement::SaveFile(std::string key, T& value){
    nlohmann::json json = JsonUtility::ToJson(value);
    
    return SaveFile(key, json.template get<std::string>());
};

//Creation
template<typename T>
bool FileManagement::CreateFile(std::filesystem::path key, T& value){
    nlohmann::json json = JsonUtility::ToJson(value);
    
    return CreateFile(key, json.template get<std::string>());
};

template<typename T>
bool FileManagement::CreateFile(std::string key, T& value){
    nlohmann::json json = JsonUtility::ToJson(value);
    
    return CreateFile(key, json.template get<std::string>());
};