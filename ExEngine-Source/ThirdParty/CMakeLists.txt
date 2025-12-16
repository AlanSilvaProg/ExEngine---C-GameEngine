project(ThirdParty)
#using GLOB to add core and essential files from thirdparties ( normally it's called include, but in this case, we don't have the include folder )
file(GLOB ENGINE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.*
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tinyfiledialogs/tinyfiledialogs.c) #needed to use input text using std::string

# Lua source files (excluding lua.c and luac.c as they contain main functions)
file(GLOB LUA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/*.c)
list(REMOVE_ITEM LUA_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.8/src/luac.c)

# adding as library all te files mapped before and specific resources like renderer as sdl in this case
add_library(ThirdParty STATIC ${ENGINE_SOURCE} ${LUA_SOURCE})

#including the imgui folder to work correctly, lua and sol folders
target_include_directories(ThirdParty PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui 
    lua-5.4.8/src 
    sol2-3.3.0/include)
#linking the sdl external resources to work correctly with the backends added previously
target_link_libraries(ThirdParty PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)