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

target_include_directories (${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})