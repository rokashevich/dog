cmake_minimum_required(VERSION 3.10)

project(dog)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "-std=gnu11")

message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
execute_process(COMMAND git -C ${CMAKE_SOURCE_DIR}
    log --pretty=format:%h -n 1 OUTPUT_VARIABLE SOURCES_VERSION)
if ("${SOURCES_VERSION}" STREQUAL "")
    set(SOURCES_VERSION "N/A")
endif()
message("SOURCES_VERSION=${SOURCES_VERSION}")
add_compile_definitions(SOURCES_VERSION="${SOURCES_VERSION}")

file(GLOB dog_sources
    "*.h"
    "*.c"
)
add_executable(dog ${dog_sources})
target_link_libraries(dog -lpthread -lrt)

add_executable(color "tests/color.c")

add_executable(segfault "tests/segfault.c")

add_executable(std "tests/std.c")
target_link_libraries(std -lpthread -lrt)
