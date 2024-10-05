set (TARGET_NAME Engine)

file (GLOB ENGINE_SOURCE Core/*.cpp Core/*.h
Core/**/*.cpp Core/**/*.h
Core/**/**/*.cpp Core/**/**/*.h)

set(ENGINE_LIBRARY_TYPE SHARED)
add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
include_directories(${TARGET_NAME} ${SDL2_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE glm::glm ${SDL2_LIBRARIES})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})