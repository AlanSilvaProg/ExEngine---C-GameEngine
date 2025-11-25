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

add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC Engine)

if(EDITOR)
target_link_libraries(${PROJECT_NAME} PUBLIC Editor)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExGameEngineWin")
elseif(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExGameEngineMac")
else()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExGameEngineLinux")
endif()

if(EDITOR)
    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
            MACOSX_BUNDLE_ICON_FILE ExGameEngineIcon256.icns
        )

        set_source_files_properties(
            ${CMAKE_CURRENT_SOURCE_DIR}/ExGameEngineIcon256.icns
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
        )

        target_sources(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ExGameEngineIcon256.icns
        )
        get_target_property(APP_TARGET_NAME ${PROJECT_NAME} OUTPUT_NAME)

        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/${APP_TARGET_NAME}.app/Contents/Resources/Assets)
        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${APP_TARGET_NAME}.app/Contents/Resources/Projects)
        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Defaults ${CMAKE_CURRENT_BINARY_DIR}/${APP_TARGET_NAME}.app/Contents/Resources)
    elseif(WIN32)
        ##Icon Windows
    else()
        ##Icon Linux
    endif()
endif()

# For Debug mode
# add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets)
# add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Projects)
# add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Defaults ${CMAKE_CURRENT_BINARY_DIR})