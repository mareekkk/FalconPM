# Make the API header visible to all plugins
include_directories(${CMAKE_SOURCE_DIR}/plugins/falconpm/src/infra)

# Hello World plugin
add_library(hello_plugin MODULE src/plugins/hello/hello_plugin.cpp)
set_target_properties(hello_plugin PROPERTIES PREFIX "" SUFFIX ".so")
