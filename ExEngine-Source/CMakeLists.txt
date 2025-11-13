cmake_minimum_required(VERSION 3.11)

project(ExEngine-Source)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
    add_compile_definitions("EXENGINE_MACOS")
    set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
elseif(WIN32)
    add_compile_definitions("EXENGINE_WINDOWS")
    set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
else()
    add_compile_definitions("EXENGINE_LINUX")
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(glm CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

option(EDITOR "Build with editor features" ON)

if(NOT DEFINED CACHE{EDITOR})
    message(STATUS "EDITOR option not provided")
    set(EDITOR ON CACHE BOOL "Build with editor features" FORCE)
else()
    message(STATUS "EDITOR explicitly set by user: ${EDITOR}")
endif()

if(EDITOR)
    add_compile_definitions("EXENGINE_EDITOR")
endif()

add_subdirectory(ThirdParty)
add_subdirectory(Code)

if(DEFINED GAME_PATH)
    message(STATUS "Building with game project at: ${GAME_PATH}")
    target_compile_definitions(Engine PUBLIC GAME_PATH="${GAME_PATH}")
else()
    message(STATUS "No external game path provided — building engine/editor only")
endif()

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)

if(EDITOR)
target_link_libraries(${PROJECT_NAME} PUBLIC Editor)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineWin")
elseif(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineMac")
else()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineLinux")
endif()

add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets)
add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Projects)
add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Defaults ${CMAKE_CURRENT_BINARY_DIR})