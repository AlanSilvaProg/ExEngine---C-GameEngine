set (TARGET_NAME Engine)

file (GLOB ENGINE_SOURCE **/*.cpp **/*.h
**/**/*.cpp **/**/*.h
**/**/**/*.cpp **/**/**/*.h
**/**/**/**/*.cpp **/**/**/**/*.h
**/**/**/**/**/*.cpp **/**/**/**/**/*.h)

set(ENGINE_LIBRARY_TYPE SHARED)
add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
target_include_directories(${TARGET_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC glm::glm ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image nlohmann_json::nlohmann_json)

# MSVC/Windows DLLs export no symbols by default. Hot-reloaded scripts (compiled standalone and
# linked against this lib) need every ECS/Component/System symbol visible, so export everything
# instead of annotating the whole Engine with __declspec(dllexport) by hand.
if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Link platform-specific libraries for stack trace symbol resolution
if(APPLE OR UNIX)
    # Link dl library for dladdr() on Unix platforms
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
elseif(WIN32)
    # Link dbghelp library for DbgHelp APIs on Windows
    target_link_libraries(${TARGET_NAME} PRIVATE dbghelp)
endif()

target_include_directories (${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})