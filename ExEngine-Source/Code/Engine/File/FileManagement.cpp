#include "FileManagement.h"
#include "../Logger/Logger.h"
#include <sstream>

bool FileManagement::LoadFileAsJson(std::string key, nlohmann::json& json){
    std::string contentAsString;

    if(!LoadFile(key, contentAsString))
        return false; 
    
    json = nlohmann::json::parse(contentAsString);
    return true;
};
    
bool FileManagement::LoadFile(std::string key, std::string& result){
    std::filesystem::path path = "";
    std::string p = ENGINE_PATH + key;
    path.append(p).replace_extension(".exfile");

    if (std::filesystem::exists(path)) {
        std::ifstream in(path, std::ios::in);
        if (in.is_open()) {
            std::stringstream buffer;
            buffer << in.rdbuf();
            result = buffer.str();

            in.close();
            Logger::Log("File" + key + " loaded from: " + path.string());

            return true;
        } else {
            Logger::LogError("File opening failed");
        }
    }
    else{
        Logger::LogError("File doesn't exist: " + key);
    }
    
    return false;
};

bool FileManagement::SaveFile(std::filesystem::path path, std::string& value){
    path.replace_extension(".exfile");

    return SaveFileAtPath(path, value);
};

bool FileManagement::SaveFile(std::string key, std::string& value){
    std::filesystem::path path = "";
    std::string p = ENGINE_PATH + key;
    path.append(p).replace_extension(".exfile");

    return SaveFileAtPath(path, value);
};

bool FileManagement::SaveFileAtPath(std::filesystem::path path, std::string& value){
    if(!std::filesystem::exists(path.parent_path().string()))
        std::filesystem::create_directories(path.parent_path().string());

    if (!std::filesystem::exists(path)) {
        std::ofstream out(path);
        if (out.is_open()) {
            out << value;
            out.close();
            Logger::Log("File created at: " + path.string());
            return true;
        } else {
            Logger::LogError("File creation failed");
        }
    } else {
        std::ofstream out(path, std::ios::trunc);
        if (out.is_open()) {
            out << value;
            out.close();
            Logger::Log("File updated successfully");
            return true;
        } else {
            Logger::LogError("File update failed");
        }
    }

    return false;
};