# Overseer plugin (the one dropped into client's plugins folder)
# Overseer plugin
add_library(overseer MODULE
    overseer.cpp
)

target_include_directories(overseer PUBLIC
    ${CMAKE_SOURCE_DIR}/src/infra
    ${CMAKE_SOURCE_DIR}/src/AI
    ${CMAKE_SOURCE_DIR}/src/core
    ${RA_SRC}
    ${RA_SRC}/map
    ${RA_SRC}/common
    ${RA_ROOT}/3rdparty/rapidyaml/src
    ${RA_ROOT}/3rdparty/rapidyaml/ext/c4core/src
    /usr/include/mariadb
)

set_target_properties(overseer PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "overseer"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
