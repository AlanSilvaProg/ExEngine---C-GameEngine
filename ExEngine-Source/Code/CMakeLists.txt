set (TARGET_NAME Code)

add_subdirectory(Engine)
add_subdirectory(Editor)

if(APPLE)
    target_compile_definitions(Engine PRIVATE EXENGINE_MACOS)
    target_compile_definitions(Editor PRIVATE EXENGINE_MACOS)
elseif(WIN32)
    target_compile_definitions(Engine PRIVATE EXENGINE_WINDOWS)
    target_compile_definitions(Editor PRIVATE EXENGINE_WINDOWS)
endif()