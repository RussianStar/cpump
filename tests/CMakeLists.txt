idf_component_register(
    SRCS "test_pump_control.c"
    INCLUDE_DIRS ".." # To access config.h and main headers
    REQUIRES app_update # Required for testing components
)

# Add test executable
add_executable(tests 
    test_pump_control.c
)
target_link_libraries(tests unity)
