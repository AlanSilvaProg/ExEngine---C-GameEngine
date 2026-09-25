set (TARGET_NAME Engine)

file (GLOB ENGINE_SOURCE **/*.cpp **/*.h
**/**/*.cpp **/**/*.h
**/**/**/*.cpp **/**/**/*.h
**/**/**/**/*.cpp **/**/**/**/*.h
**/**/**/**/**/*.cpp **/**/**/**/**/*.h)

# A wasm module has no real notion of a shared library the way desktop does, and this CMake/
# Emscripten combination doesn't support SHARED there anyway - build Engine as STATIC for web.
if(EMSCRIPTEN)
    set(ENGINE_LIBRARY_TYPE STATIC)
else()
    set(ENGINE_LIBRARY_TYPE SHARED)
endif()
add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
target_include_directories(${TARGET_NAME} PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC nlohmann_json::nlohmann_json)

if(NOT EMSCRIPTEN)
    # glm's imported target and the SDL2_ttf/SDL2_image ones only exist when the Homebrew
    # find_package calls above actually ran - an Emscripten build sources these through the
    # compiler/linker -s flags set at the root CMakeLists.txt instead.
    target_link_libraries(${TARGET_NAME} PUBLIC glm::glm ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)
endif()

if(EMSCRIPTEN)
    # emscripten_websocket_* (BidirectionalCommunication's EmscriptenWebSocketConnection) needs
    # the WebSocket JS library pulled in explicitly - it is not linked by default.
    target_link_options(${TARGET_NAME} PUBLIC -lwebsocket.js)
else()
    # Native-only network backends: TCPWebSocketConnection (libcurl) and UDPQUICConnection
    # (msquic) - neither can run inside the browser sandbox.
    target_link_libraries(${TARGET_NAME} PUBLIC CURL::libcurl msquic::msquic)
endif()

# MSVC/Windows DLLs export no symbols by default. Hot-reloaded scripts (compiled standalone and
# linked against this lib) need every ECS/Component/System symbol visible, so export everything
# instead of annotating the whole Engine with __declspec(dllexport) by hand.
if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Link platform-specific libraries for stack trace symbol resolution
if((APPLE OR UNIX) AND NOT EMSCRIPTEN)
    # Link dl library for dladdr() on Unix platforms
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
elseif(WIN32)
    # Link dbghelp library for DbgHelp APIs on Windows
    target_link_libraries(${TARGET_NAME} PRIVATE dbghelp)
endif()

target_include_directories (${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})