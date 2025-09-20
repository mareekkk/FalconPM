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
    ${CMAKE_SOURCE_DIR}/src/infra
)
target_link_libraries(falconpm_loader PRIVATE dl)

set_target_properties(falconpm_loader PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "falconpm_loader"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# -------------------------------
# 2) FalconPM base (ABI context)
# -------------------------------
add_library(falconpm_base MODULE
    src/core/falconpm.cpp
    src/infra/plugin_api.h
)
target_include_directories(falconpm_base PUBLIC
    ${CMAKE_SOURCE_DIR}/src/infra
    ${RA_SRC}
    ${RA_SRC}/map
    ${RA_SRC}/common
    ${RA_ROOT}/3rdparty/rapidyaml/src
    ${RA_ROOT}/3rdparty/rapidyaml/ext/c4core/src
    /usr/include/mariadb    
)
set_target_properties(falconpm_base PROPERTIES
    PREFIX ""
    SUFFIX ".so"
    OUTPUT_NAME "falconpm_base"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

# -------------------------------
# 3) Autoroute plugin (test plugin)
# -------------------------------
# -------------------------------
# 3) Autoroute plugin (test plugin)
# -------------------------------
add_library(autoroute MODULE
    src/plugins/autoroute/autoroute.cpp
    src/core/fpm_path.c
    src/core/fpm_graph.c
    src/core/fpm_route.c
    src/core/fpm_portals.c
    src/infra/plugin_api.h
)

target_include_directories(autoroute PUBLIC
    ${CMAKE_SOURCE_DIR}/src/infra
    ${RA_SRC}          # <-- rAthena src root
    ${RA_SRC}/map      # <-- for map/ headers
    ${RA_SRC}/common   # <-- for common/ headers
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

target_link_options(falconpm_base PRIVATE -Wl,--export-dynamic)
