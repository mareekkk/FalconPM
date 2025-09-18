add_library(loader STATIC loader.cpp)
target_link_libraries(loader common)

add_library(hello_plugin MODULE ../../plugins/hello/hello_plugin.c)
set_target_properties(hello_plugin PROPERTIES PREFIX "" SUFFIX ".so")
target_link_libraries(hello_plugin loader)
