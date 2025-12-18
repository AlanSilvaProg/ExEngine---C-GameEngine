#include "FileManagement.h"
#include "../Core/Engine.h"
#include "../Logger/Logger.h"
#include <sstream>

//Save & Load
bool FileManagement::LoadFileAsJson(std::string key, nlohmann::json& json){
    std::string contentAsString;

    if(!LoadFile(key, contentAsString))
        return false; 
    
    json = nlohmann::json::parse(contentAsString);

    if(json.is_null() || json.empty())
    {
        Logger::LogError("Null or empty json as result from" + key);
    }
    
    return true;
};

bool FileManagement::LoadFileAsJson(std::filesystem::path path, nlohmann::json& json){
    std::string contentAsString;

    if(!LoadFile(path, contentAsString))
        return false; 
    
    json = nlohmann::json::parse(contentAsString);

    if(json.is_null() || json.empty())
    {
        Logger::LogError("Null or empty json as result from: " + path.string());
    }
    
    return true;
};
    
bool FileManagement::LoadFile(std::string key, std::string& result){
    std::filesystem::path path = "";
    std::string p = Engine::GetEnginePath() / key;
    path.append(p);

    ValidateExtension(path);

    if (std::filesystem::exists(path.parent_path())) {
        std::ifstream in(path, std::ios::in);
        if (in.is_open()) {
            std::stringstream buffer;
            buffer << in.rdbuf();
            result = buffer.str();

            in.close();
            Logger::Log("File " + key + " loaded from: " + path.string());

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
    
bool FileManagement::LoadFile(std::filesystem::path path, std::string& result){
    ValidateExtension(path);

    if (std::filesystem::exists(path.parent_path())) {
        std::ifstream in(path, std::ios::in);
        if (in.is_open()) {
            std::stringstream buffer;
            buffer << in.rdbuf();
            result = buffer.str();

            in.close();
            Logger::Log("File loaded: " + path.string());

            return true;
        } else {
            Logger::LogError("File opening failed");
        }
    }
    else{
        Logger::LogError("File doesn't exist: " + path.string());
    }
    
    return false;
};

bool FileManagement::SaveFile(std::filesystem::path path, std::string value){
    return SaveFileAtPath(path, value.c_str());
};

bool FileManagement::SaveFile(std::string key, std::string value){
    return SaveFile(key, value.c_str());
};

bool FileManagement::SaveFile(std::filesystem::path path, const char* value){
    return SaveFileAtPath(path, value);
};

bool FileManagement::SaveFile(std::string key, const char* value){
    std::filesystem::path path = "";
    std::string p = Engine::GetEnginePath() / key;
    path.append(p);

    return SaveFileAtPath(path, value);
};

bool FileManagement::SaveFileAtPath(std::filesystem::path path, std::string& value){
    return SaveFileAtPath(path, value.c_str());
};

bool FileManagement::SaveFileAtPath(std::filesystem::path path, const char* value)
{
    ValidateExtension(path);

    if(!std::filesystem::exists(path.parent_path()))
        std::filesystem::create_directories(path.parent_path());

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
}

//Creation

bool FileManagement::CreateFile(std::filesystem::path path, std::string value){
    return CreateFileAtPath(path, value.c_str());
};

bool FileManagement::CreateFile(std::string key, std::string value){
    return CreateFile(key, value.c_str());
};

bool FileManagement::CreateFile(std::filesystem::path path, const char* value){
    return CreateFileAtPath(path, value);
};

bool FileManagement::CreateFile(std::string key, const char* value){
    std::filesystem::path path = "";
    std::string p = Engine::GetEnginePath() / key;
    path.append(p);

    return CreateFileAtPath(path, value);
};

bool FileManagement::CreateFileAtPath(std::filesystem::path path, std::string& value){
    return CreateFileAtPath(path, value.c_str());
};

bool FileManagement::CreateFileAtPath(std::filesystem::path path, const char* value)
{
    ValidateExtension(path);

    std::filesystem::path dir = path.parent_path();
    std::string stem = path.stem().string();      
    std::string ext = path.extension().string(); 

    int number = 0;
    std::filesystem::path newPath;

    do
    {
        if (number == 0)
            newPath = path;
        else
            newPath = dir / (stem + "(" + std::to_string(number) + ")" + ext);

        number++;
    }
    while (std::filesystem::exists(newPath));

    std::ofstream out(newPath);
    if (!out.is_open())
    {
        Logger::LogError("File creation failed");
        return false;
    }

    out << value;
    out.close();

    Logger::Log("File created at: " + newPath.string());
    return true;
};

void FileManagement::ValidateExtension(std::filesystem::path& path){
    if(!path.has_extension()){
        path.replace_extension(".exfile");
    }
};