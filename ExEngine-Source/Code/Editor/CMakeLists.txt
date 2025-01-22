set (TARGET_NAME Editor)

file(GLOB ENGINE_SOURCE */**.h */**.cpp)

set(ENGINE_LIBRARY_TYPE SHARED)

add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
target_link_libraries(${TARGET_NAME} PRIVATE ThirdParty)