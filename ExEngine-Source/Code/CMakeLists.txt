set (TARGET_NAME Code)

add_subdirectory(Engine)

if(EDITOR)
    add_subdirectory(Editor)
endif()