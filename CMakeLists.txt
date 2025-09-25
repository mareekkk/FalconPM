cmake_minimum_required(VERSION 3.25)

# Project definition
project(FalconPM)
set(CMAKE_CXX_STANDARD 17)

# Silence relative path warnings for target_sources
cmake_policy(SET CMP0076 NEW)

# -------------------------------
# FalconPM base (ABI context)
# -------------------------------
add_library(falconpm_base SHARED)

# Global include paths (shared by all modules)
target_include_directories(falconpm_base PUBLIC
    ${CMAKE_SOURCE_DIR}/src/infra
    ${RA_SRC}
    ${RA_SRC}/map
    ${RA_SRC}/common
    ${RA_ROOT}/3rdparty/rapidyaml/src
    ${RA_ROOT}/3rdparty/rapidyaml/ext/c4core/src
    /usr/include/mariadb
    ${CMAKE_SOURCE_DIR}/src/AI
    ${CMAKE_SOURCE_DIR}/src/core
)

set_target_properties(falconpm_base PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "falconpm_base"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
target_link_options(falconpm_base PRIVATE -Wl,--export-dynamic)

# -------------------------------
# Subdirectories (sources added via target_sources)
# -------------------------------
add_subdirectory(src/core)
add_subdirectory(src/infra)
add_subdirectory(src/AI/peregrine)
add_subdirectory(src/AI/merlin)
add_subdirectory(src/AI/lanner)
add_subdirectory(src/AI/hunter)
add_subdirectory(src/plugins)   # overseer plugin
