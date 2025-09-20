cmake_minimum_required(VERSION 3.10)
project(FalconPM)
set(CMAKE_CXX_STANDARD 17)

# rAthena source root
set(RA_ROOT "/home/marek/rathena")
set(RA_SRC  "${RA_ROOT}/src")
set(RA_3P_RYML "${RA_ROOT}/3rdparty/rapidyaml/src")
set(RA_3P_C4   "${RA_ROOT}/3rdparty/rapidyaml/ext/c4core/src")

# -------------------------------
# 1) Loader (MODULE, preloaded by map-server)
# -------------------------------
add_library(falconpm_loader MODULE
    src/infra/loader.cpp
    src/infra/plugin_api.h
)
target_include_directories(falconpm_loader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra
)
target_link_libraries(falconpm_loader PRIVATE dl)

set_target_properties(falconpm_loader PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "falconpm_loader"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# -------------------------------
# 2) FalconPM base (MODULE, loaded by loader)
# -------------------------------
add_library(falconpm_base MODULE
    src/core/falconpm.cpp   # your "heart" file
    src/infra/plugin_api.h
)
target_include_directories(falconpm_base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra
    ${RA_SRC}
)
set_target_properties(falconpm_base PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "falconpm_base"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# -------------------------------
# 3) Autoroute plugin (MODULE, loaded by loader)
# -------------------------------
add_library(autoroute MODULE
    src/plugins/autoroute/autoroute.cpp
)
target_include_directories(autoroute PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infra
    ${RA_SRC}
    ${RA_SRC}/map
    ${RA_SRC}/common
    ${RA_3P_RYML}
    ${RA_3P_C4}
    /usr/include/mariadb
)
set_target_properties(autoroute PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "autoroute"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)
