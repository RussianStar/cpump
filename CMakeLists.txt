cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(pump_qwq)

set(EXTRA_COMPONENT_DIRS "tests/unity")

idf_component_register(
    SRCS "main/main.c"
    INCLUDE_DIRS "main"
    REQUIRES espnow
)

# Add Unity test component
add_subdirectory(tests/unity)
