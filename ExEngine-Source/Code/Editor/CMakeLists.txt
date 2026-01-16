set (TARGET_NAME Editor)

file(GLOB ENGINE_SOURCE */**.h */**.cpp
                */**/**.h */**/**.cpp
                */**/**/**.h */**/**/**.cpp
                */**/**/**/**.h */**/**/**/**.cpp
                */**/**/**/**/**.h */**/**/**/**/**.cpp)

set(ENGINE_LIBRARY_TYPE SHARED)

add_library(${TARGET_NAME} ${ENGINE_LIBRARY_TYPE} ${ENGINE_SOURCE})
target_link_libraries(${TARGET_NAME} PRIVATE ThirdParty Engine)

# Link macOS frameworks for FileWatcher
if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE 
        "-framework CoreFoundation"
        "-framework CoreServices"
    )
endif()