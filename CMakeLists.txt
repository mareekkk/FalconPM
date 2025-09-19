cmake_minimum_required(VERSION 3.10)
project(falconpm)

set(CMAKE_CXX_STANDARD 17)

# ------------------------------------------------------
# 1. Build the FalconPM loader (shared lib)
# ------------------------------------------------------
add_library(falconpm_loader SHARED
    src/infra/loader.cpp
)

target_include_directories(falconpm_loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra
)

set_target_properties(falconpm_loader PROPERTIES
    PREFIX ""                      # no "lib" prefix
    OUTPUT_NAME "falconpm_loader"  # output filename
)

# ------------------------------------------------------
# 2. Build hello_plugin (example plugin)
# ------------------------------------------------------
add_library(hello_plugin MODULE
    src/plugins/hello_plugin.c
)

target_include_directories(hello_plugin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra
)

set_target_properties(hello_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "hello_plugin"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# ------------------------------------------------------
# (Optional) If you add more plugins later:
#   add_library(my_plugin MODULE src/plugins/my_plugin.c)
#   set_target_properties(my_plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
# ------------------------------------------------------
