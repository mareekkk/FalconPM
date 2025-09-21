#pragma once
#include "pgn_path.h"
#include "map.hpp"
#include "../infra/plugin_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    PStepList steps;                 // Path returned by Peregrine A*
    int index;                       // Current step index
    struct map_session_data* sd;     // Player session
    bool active;                     // Is routing active?
} PeregrineAI;

// Start a new route
void pgn_route_start(const PluginContext* ctx,
                     struct map_session_data* sd,
                     PStepList* steps);

// Stop the current route
void pgn_route_stop(void);

// Check if a route is active
bool pgn_route_active(void);

#ifdef __cplusplus
}
#endif
