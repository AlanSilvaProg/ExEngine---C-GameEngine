cmake_minimum_required(VERSION 3.11)

project(Build_Release_CLI-Source)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file (GLOB Build_Release_CLI-Source **/*.cpp **/*.h)

add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Build_Release_CLI")
target_compile_definitions(Build_Release_CLI-Source PUBLIC CLI_BIN_PATH=\"${CMAKE_BINARY_DIR}\")