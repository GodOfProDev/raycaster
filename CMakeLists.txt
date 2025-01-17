cmake_minimum_required(VERSION 3.22)
project(raycast)

set(CMAKE_C_STANDARD 23)

#set(SOURCES
#    "src/instantcg.h"
#    "src/instantcg.cpp"
#    "src/main.cpp"
#)

set(SOURCES
    "src/main.c" 
    "src/util.h"
 "src/types.h")

# Add your source files
add_executable(raycast ${SOURCES})

# Add the path to SDL2 headers
target_include_directories(raycast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/include)

# Add the path to SDL2 libraries
target_link_directories(raycast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/lib/x64)

# Link SDL2 libraries
target_link_libraries(raycast SDL2main SDL2 SDL2_Image SDL2_ttf)

# Copy SDL2.dll to the output directory (for Windows)
if(WIN32)
    foreach(DLL IN ITEMS SDL2.dll SDL2_image.dll SDL2_ttf.dll)
        add_custom_command(TARGET raycast POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/vendor/SDL/lib/x64/${DLL}"
                $<TARGET_FILE_DIR:raycast>)
    endforeach()
endif()