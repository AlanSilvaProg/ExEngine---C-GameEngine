project(ThirdParty)

# ImGui and TinyFileDialogs source files (Editor only)
file(GLOB EDITOR_UI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.*
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/tinyfiledialogs/tinyfiledialogs.c) #needed to use input text using std::string

# Create separate libraries
# UI library for Editor only
add_library(ThirdParty_UI STATIC ${EDITOR_UI_SOURCE})

# Main ThirdParty library (for backward compatibility)
add_library(ThirdParty STATIC ${EDITOR_UI_SOURCE})

# Configure UI library
target_include_directories(ThirdParty_UI PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui)

# Configure main ThirdParty library (backward compatibility)
target_include_directories(ThirdParty PUBLIC . ${CMAKE_CURRENT_SOURCE_DIR} imgui)

if(NOT EMSCRIPTEN)
    # SDL2_ttf::SDL2_image are Homebrew imported targets that only exist when the root
    # find_package calls ran - an Emscripten build sources SDL2 through the -s flags set at the
    # root CMakeLists.txt instead, applied to every target already.
    target_link_libraries(ThirdParty_UI PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)
    target_link_libraries(ThirdParty PUBLIC ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)
endif()