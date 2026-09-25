cmake_minimum_required(VERSION 3.11)

project(ExEngine-Source)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Configure debug symbols for stack traces
# Debug builds: Full debug information with frame pointers
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(APPLE OR UNIX)
        add_compile_options(-g -fno-omit-frame-pointer)
    elseif(WIN32)
        add_compile_options(/Zi)
    endif()
endif()

# Release builds: Minimal debug info for function names in stack traces
if(CMAKE_BUILD_TYPE MATCHES Release)
    if(APPLE OR UNIX)
        add_compile_options(-g1)
    endif()
endif()

if(APPLE)
    add_compile_definitions("EXENGINE_MACOS")
    set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
elseif(WIN32)
    add_compile_definitions("EXENGINE_WINDOWS")
    set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
elseif(EMSCRIPTEN)
    add_compile_definitions("EXENGINE_WEB")
    # glm is header-only; its CMake config package lives under the Homebrew (host) prefix, which
    # the Emscripten toolchain's find_package (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY) can never
    # see - reuse the same installed headers directly instead of going through find_package below.
    set(GLM_INCLUDE_DIRS /opt/homebrew/Cellar/glm/1.0.0/include)
else()
    add_compile_definitions("EXENGINE_LINUX")
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

# glm's config package and the SDL2/SDL2_ttf/SDL2_image Homebrew binaries are all native (host)
# artifacts - none of it can link into a .wasm module, and an Emscripten build sources SDL2
# through its own bundled ports instead (see the EMSCRIPTEN block below), so skip all four here.
if(NOT EMSCRIPTEN)
    find_package(glm CONFIG REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_image REQUIRED)
endif()

if(EMSCRIPTEN)
    # Emscripten's bundled SDL2/SDL2_ttf/SDL2_image ports replace the Homebrew find_package calls
    # above. These flags must be visible at both compile time (so <SDL.h> etc. resolve to the
    # port's headers) and link time (to pull the port library in), so apply them to every target.
    set(EMSCRIPTEN_SDL_FLAGS -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_IMAGE=2)
    add_compile_options(${EMSCRIPTEN_SDL_FLAGS})
    add_link_options(${EMSCRIPTEN_SDL_FLAGS})

    # emscripten_fetch (NetworkRequest's Emscripten backend) needs its JS support code pulled in
    # explicitly, same as -lwebsocket.js above.
    add_compile_options(-sFETCH=1)
    add_link_options(-sFETCH=1)

    # NetworkRequest's Emscripten backend (emscripten_fetch) blocks the calling thread until the
    # request completes via EMSCRIPTEN_FETCH_SYNCHRONOUS, which only works off the browser's main
    # thread - real Web Workers, which -pthread turns std::thread into. Every current caller
    # already runs network requests from a background std::thread, so this matches. Note this
    # requires the page to be served with the Cross-Origin-Opener-Policy/Cross-Origin-Embedder-Policy
    # headers SharedArrayBuffer needs, or thread creation fails at runtime.
    add_compile_options(-pthread)
    add_link_options(-pthread)
endif()

# CURL and msquic back the native TCPWebSocketConnection/UDPQUICConnection transports, neither
# of which can run inside the browser sandbox - an Emscripten build uses EmscriptenWebSocketConnection
# instead (see BidirectionalCommunication/Emscripten/EmscriptenWebSocketConnection.h), so skip both.
if(NOT EMSCRIPTEN)
    # macOS ships a system libcurl (linked by default), but it is built without WebSocket
    # and HTTP/3 support. Point CMake at the Homebrew curl keg instead, which has both -
    # needed for BidirectionalCommunication's TCPWebSocketConnection/UDPQUICConnection.
    if(APPLE)
        set(CURL_INCLUDE_DIR /opt/homebrew/opt/curl/include)
        set(CURL_LIBRARY /opt/homebrew/opt/curl/lib/libcurl.dylib)
    endif()
    find_package(CURL REQUIRED)

    # msquic (installed via `brew install libmsquic`) provides the QUIC/HTTP3 backend for
    # UDPQUICConnection in BidirectionalCommunication.
    if(APPLE)
        set(msquic_DIR /opt/homebrew/opt/libmsquic/share/msquic)
    endif()
    find_package(msquic CONFIG REQUIRED)
endif()

#Editor option
option(EDITOR "Build with editor features" ON)

if(NOT DEFINED CACHE{EDITOR})
    message(STATUS "EDITOR option not provided")
    set(EDITOR ON CACHE BOOL "Build with editor features" FORCE)
else()
    message(STATUS "EDITOR explicitly set by user: ${EDITOR}")
endif()

if(EDITOR)
    add_compile_definitions("EXENGINE_EDITOR")
else()
    # App::isPlaying (Code/Engine/Core/Runtime/App.cpp) is only ever flipped to true by the
    # Editor's own Play button (GameShortcutsWindow.cpp) - a shipped build has no separate edit/play
    # mode, so it needs to start "playing" on its own. App.cpp already has this exact branch, just
    # never had anything defining GAME_BUILD to take it.
    add_compile_definitions("GAME_BUILD")
endif()
#

#Debug mode Option
option(DEBUG_MODE "Build with DEBUG_MODE features" OFF)

if(NOT DEFINED CACHE{DEBUG_MODE})
    message(STATUS "Debug mode option not provided")
    set(DEBUG_MODE OFF CACHE BOOL "Build with Debug mode features" FORCE)
else()
    message(STATUS "Debug mode explicitly set by user: ${DEBUG_MODE}")
endif()

if(DEBUG_MODE)
    add_compile_definitions("EXENGINE_DEBUG_MODE")
endif()
#

# Single flag meaning "this is a dev build" - true for the Editor (always) and for a shipped
# (non-editor) build exported with Dev Mode on (PlayerBuilder passes -DDEBUG_MODE=ON for those,
# OFF for Release). Logger.cpp gates its output on this alone instead of re-deriving the same
# EDITOR-or-DEBUG_MODE condition itself.
if(EDITOR OR DEBUG_MODE)
    add_compile_definitions("EXENGINE_BUILD_DEV")
endif()
#

add_subdirectory(ThirdParty)
add_subdirectory(Code)

if(DEFINED GAME_PATH)
    message(STATUS "Building with game project at: ${GAME_PATH}")
    target_compile_definitions(Engine PUBLIC GAME_PATH="${GAME_PATH}")

    if(NOT EDITOR)
        # A shipped (non-editor) build has no ScriptHotReloadManager/dlopen to load a project's
        # scripts at runtime - bake them into Engine instead. REGISTER_COMPONENT/REGISTER_SYSTEM
        # (Code/Engine/Core/ECS/InternalRegistry/ComponentRegistry.h, SystemRegistry.h) self-register
        # via a static initializer, so simply compiling each script as a normal translation unit
        # reproduces the same registration that dlopen()'ing it does today in the Editor. This
        # mirrors ScriptCompiler::Compile()'s own trampoline trick (Code/Editor/Scripting/
        # ScriptCompiler.cpp) instead of compiling the .hpp directly, since project scripts use
        # repo-root-relative includes like "Code/Engine/ExEngine.h".
        target_include_directories(Engine PUBLIC ${CMAKE_SOURCE_DIR})

        file(GLOB_RECURSE GAME_SCRIPT_HEADERS "${GAME_PATH}/Assets/Scripts/*.hpp")
        foreach(scriptHeader ${GAME_SCRIPT_HEADERS})
            get_filename_component(scriptName ${scriptHeader} NAME_WE)
            set(trampolinePath "${CMAKE_BINARY_DIR}/GeneratedScripts/${scriptName}.generated.cpp")
            file(WRITE ${trampolinePath} "#include \"${scriptHeader}\"\n")
            target_sources(Engine PRIVATE ${trampolinePath})
        endforeach()

        if(GAME_SCRIPT_HEADERS)
            list(LENGTH GAME_SCRIPT_HEADERS GAME_SCRIPT_COUNT)
            message(STATUS "Baked ${GAME_SCRIPT_COUNT} game script(s) from ${GAME_PATH}/Assets/Scripts into Engine")
        endif()
    endif()
else()
    message(STATUS "No external game path provided — building engine/editor only")
endif()


if(APPLE AND NOT DEBUG_MODE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE main.cpp)
else()
    add_executable(${PROJECT_NAME} main.cpp)
endif()

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
elseif(EMSCRIPTEN)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExGameEngineWeb" SUFFIX ".html")
else()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ExGameEngineLinux")
endif()

if(DEFINED GAME_PATH AND NOT EDITOR AND EXISTS "${GAME_PATH}/Assets")
    # Bundle the project's Assets next to the executable, in the same "Assets" folder
    # Engine::GetEngineAssetsPath() (SDL_GetBasePath()/Assets) already looks in, so asset-loading
    # code needs no changes. SDL resolves GetBasePath() to Contents/Resources inside a macOS .app
    # bundle rather than the folder holding the Mach-O binary, so a bundled build copies there.
    if(APPLE AND NOT DEBUG_MODE)
        set(GAME_ASSETS_DEST "$<TARGET_BUNDLE_CONTENT_DIR:${PROJECT_NAME}>/Resources/Assets")
    else()
        set(GAME_ASSETS_DEST "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets")
    endif()

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${GAME_PATH}/Assets" "${GAME_ASSETS_DEST}")

    if(EMSCRIPTEN)
        # That plain directory isn't reachable from inside the browser sandbox - preload it into
        # the wasm virtual filesystem at the same relative path instead.
        target_link_options(${PROJECT_NAME} PRIVATE "--preload-file" "${GAME_PATH}/Assets@Assets")
    endif()
endif()

if(EDITOR)
    if(APPLE)
        if(DEBUG_MODE)
            add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets)
            add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Projects)
            add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Defaults ${CMAKE_CURRENT_BINARY_DIR})
        else()
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
        endif()
    else()
        if(WIN32)
            ##Icon Windows
        else()
            ##Icon Linux
        endif()

        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets)
        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Projects)
        add_custom_command(TARGET ExEngine-Source PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Defaults ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endif()