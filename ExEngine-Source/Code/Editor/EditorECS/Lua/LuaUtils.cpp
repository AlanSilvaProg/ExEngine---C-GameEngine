#include "LuaUtils.h"
#include "../Engine/File/FileManagement.h"

void LuaUtils::CreateLuaECSystem(std::filesystem::path path){
    FileManagement::CreateFile(path, BASE_LUA_ECSYSTEM);
};

void LuaUtils::CreateLuaEComponenteS(std::filesystem::path path){
    FileManagement::CreateFile(path, BASE_LUA_ECOMPONENTS);
};