set (TARGET_NAME Engine)

file (GLOB ENGINE_SOURCE Core/*.cpp Core/*.h
Core/**/*.cpp Core/**/*.h
Core/**/**/*.cpp Core/**/**/*.h)

set(ENGINE_LIBRARY_TYPE SHARED)
add_library(Engine ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})

include_directories (${CMAKE_CURRENT_SOURCE_DIR})