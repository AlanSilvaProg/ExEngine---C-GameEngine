project(ThirdParty)

# Lua source files (excluding lua.c and luac.c as they contain main functions)
file(GLOB LUA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/*.c)
list(REMOVE_ITEM LUA_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/luac.c)

# ImGui and TinyFileDialogs source files (Editor only)
file(GLOB EDITOR_UI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.*
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tinyfiledialogs/tinyfiledialogs.c) #needed to use input text using std::string

# Create separate libraries
# Lua library for Engine and Editor
add_library(ThirdParty_Lua STATIC ${LUA_SOURCE})

# UI library for Editor only
add_library(ThirdParty_UI STATIC ${EDITOR_UI_SOURCE})

# Main ThirdParty library (for backward compatibility)
add_library(ThirdParty STATIC ${LUA_SOURCE} ${EDITOR_UI_SOURCE})

# Configure Lua library
target_include_directories(ThirdParty_Lua PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} 
    lua-5.4.8/src 
    sol2-3.5.0/include)

# Ensure Lua is compiled with proper C standard and long long support
# Force include limits.h to ensure LLONG_MAX is defined for C++26 compatibility
if(APPLE OR UNIX)
    target_compile_definitions(ThirdParty_Lua PUBLIC LUA_USE_MACOSX)
    target_compile_definitions(ThirdParty_Lua PUBLIC LLONG_MAX)
elseif(WIN32)
    target_compile_definitions(ThirdParty_Lua PUBLIC LUA_USE_WINDOWS)
endif()

# Configure UI library
target_include_directories(ThirdParty_UI PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui)
target_link_libraries(ThirdParty_UI PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)

# Configure main ThirdParty library (backward compatibility)
target_include_directories(ThirdParty PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui 
    lua-5.4.8/src 
    sol2-3.5.0/include)

target_link_libraries(ThirdParty PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)

# Apply Lua platform definitions to main ThirdParty library too
if(APPLE OR UNIX)
    target_compile_definitions(ThirdParty PUBLIC LUA_USE_MACOSX)
elseif(WIN32)
    target_compile_definitions(ThirdParty PUBLIC LUA_USE_WINDOWS)
endif()