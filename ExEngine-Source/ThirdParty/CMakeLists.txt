project(ThirdParty)

#using GLOB to add core and essential files from thirdparties ( normally it's called include, but in this case, we don't have the include folder )
file(GLOB ENGINE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp) 

# adding as library all te files mapped before and specific resources like renderer as sdl in this case
add_library(ThirdParty STATIC ${ENGINE_SOURCE}
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp)

#including de imgui folder to work correctly
target_include_directories(ThirdParty PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui)
#linking the sdl external resources to work correctly with the backends added previously
target_link_libraries(ThirdParty PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)