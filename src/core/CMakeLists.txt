# Core sources (server bridge, tick, pathfinding shims, etc.)
target_sources(falconpm_base PRIVATE
    falconpm.cpp
    fpm_graph.c
    fpm_menu.cpp
    fpm_path.c
    fpm_portals.c
    fpm_route.c
    fpm_tick.cpp
    ../AI/peregrine/pgn_api.c
    ../AI/merlin/mln_api.c
)
