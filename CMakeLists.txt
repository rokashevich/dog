cmake_minimum_required(VERSION 2.8)

project(dog)

file(GLOB dog_sources
    "*.h"
    "*.c"
)

execute_process(COMMAND git log --pretty=format:'%h' -n 1
                OUTPUT_VARIABLE SOURCES_VERSION
                ERROR_QUIET)
if ("${SOURCES_VERSION}" STREQUAL "")
    set(SOURCES_VERSION "N/A")
endif()
add_compile_definitions(SOURCES_VERSION="${SOURCES_VERSION}")

add_executable(dog ${dog_sources})
target_link_libraries(dog -lpthread -lrt)
