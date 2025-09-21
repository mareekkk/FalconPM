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
    src/AI/peregrine_ai.c
    src/AI/peregrine_gat.c      # <-- add this
    src/AI/peregrine_path.c     # <-- and this
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
target_link_options(falconpm_base PRIVATE -Wl,--export-dynamic)

# -------------------------------
# 3) Autoroute plugin (test plugin + Peregrine AI)
# -------------------------------
add_library(autoroute MODULE
    src/plugins/autoroute/autoroute.cpp
    src/core/fpm_path.c
    src/core/fpm_graph.c
    src/core/fpm_route.c
    src/core/fpm_portals.c
    src/AI/peregrine_gat.c
    src/AI/peregrine_path.c
    src/AI/peregrine_ai.c      # <-- new orchestrator
    # later: src/AI/peregrine_portals.c
    # later: src/AI/peregrine_route.c
    src/infra/plugin_api.h
)

target_include_directories(autoroute PUBLIC
    ${CMAKE_SOURCE_DIR}/src/infra
    ${CMAKE_SOURCE_DIR}/src/AI          # <-- include Peregrine headers
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

# -------------------------------
# 4) Peregrine test executables
# -------------------------------

# Test GAT loader
add_executable(gat_test
    src/AI/tests/main_gat_test.c
    src/AI/peregrine_gat.c
)
target_include_directories(gat_test PUBLIC
    ${CMAKE_SOURCE_DIR}/src/AI
)
set_target_properties(gat_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Test Pathfinding (A*)
add_executable(path_test
    src/AI/tests/main_path_test.c
    src/AI/peregrine_gat.c
    src/AI/peregrine_path.c
)
target_include_directories(path_test PUBLIC
    ${CMAKE_SOURCE_DIR}/src/AI
)
set_target_properties(path_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)
