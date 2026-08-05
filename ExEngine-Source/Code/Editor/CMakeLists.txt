set (TARGET_NAME Editor)

file(GLOB ENGINE_SOURCE */**.h */**.cpp
                */**/**.h */**/**.cpp
                */**/**/**.h */**/**/**.cpp
                */**/**/**/**.h */**/**/**/**.cpp
                */**/**/**/**/**.h */**/**/**/**/**.cpp)

set(ENGINE_LIBRARY_TYPE SHARED)

add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
target_link_libraries(${TARGET_NAME} PRIVATE ThirdParty Engine)

# Link macOS frameworks for FileWatcher
if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework CoreFoundation"
        "-framework CoreServices"
    )
endif()

if(UNIX)
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
endif()

# Bake the flags needed to compile a single user script (.hpp) into a standalone shared library
# and link it against this same Engine build, so the ScriptCompiler doesn't have to guess/replicate
# the toolchain used to build the editor itself. Values are '|'-separated where more than one path
# can be present, since CMake lists collapse to ';' which the Editor's own args already use.
set(SCRIPT_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code/Engine
    ${json_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)
list(JOIN SCRIPT_INCLUDE_DIRS "|" SCRIPT_INCLUDE_DIRS_STR)

set(SCRIPT_DEFINES_LIST "EXENGINE_EDITOR")
if(APPLE)
    list(APPEND SCRIPT_DEFINES_LIST "EXENGINE_MACOS")
elseif(WIN32)
    list(APPEND SCRIPT_DEFINES_LIST "EXENGINE_WINDOWS")
else()
    list(APPEND SCRIPT_DEFINES_LIST "EXENGINE_LINUX")
endif()
list(JOIN SCRIPT_DEFINES_LIST "|" SCRIPT_DEFINES_STR)

target_compile_definitions(${TARGET_NAME} PRIVATE
    SCRIPT_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
    SCRIPT_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
    SCRIPT_CXX_STANDARD="${CMAKE_CXX_STANDARD}"
    SCRIPT_ENGINE_LINK_LIB="$<TARGET_LINKER_FILE:Engine>"
    SCRIPT_INCLUDE_DIRS="${SCRIPT_INCLUDE_DIRS_STR}"
    SCRIPT_DEFINES="${SCRIPT_DEFINES_STR}"
)