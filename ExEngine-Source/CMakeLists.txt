cmake_minimum_required(VERSION 3.11)

project(ExEngine-Source)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
elseif(WIN32)
set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
endif()

find_package(glm CONFIG REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})

add_subdirectory(Code)
add_subdirectory(ThirdParty)
add_executable(${PROJECT_NAME} main.cpp)

include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Engine)

if(WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineWin")
elseif(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineMac")
else()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExEngineLinux")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")