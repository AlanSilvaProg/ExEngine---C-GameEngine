set (TARGET_NAME Engine)

file (GLOB ENGINE_SOURCE Core/*.cpp Core/*.h
Core/**/*.cpp Core/**/*.h
Core/**/**/*.cpp Core/**/**/*.h
GameCore/*.cpp GameCore/*.h
GameCore/**/*.cpp GameCore/**/*.h
Logger/*.cpp Logger/*.h 
Resource/*.cpp Resource/*.h)

set(ENGINE_LIBRARY_TYPE SHARED)
add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
include_directories(${TARGET_NAME} ${SDL2_INCLUDE_DIRS} ${SDL2_ttf_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC glm::glm ${SDL2_LIBRARIES} SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(Engine PUBLIC ASSETS_PATH="${CMAKE_BINARY_DIR}/Assets")