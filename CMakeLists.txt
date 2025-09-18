# FalconPM CMake — builds plugins only (loader is compiled into map-server)

# Include API header for plugins
include_directories(${CMAKE_SOURCE_DIR}/plugins/falconpm/src/infra)

# Hello World plugin
add_library(hello_plugin MODULE src/plugins/hello/hello_plugin.cpp)
set_target_properties(hello_plugin PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "hello_plugin"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
