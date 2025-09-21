#pragma once
#include "pgn_path.h"
#include "pgn_gat.h"
#include "../../infra/plugin_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (instead of including map.hpp)
struct map_session_data;
struct GatMap;
struct PStepList;

// Forward declare rAthena session
struct map_session_data;

// ----------------------------------------------------
// PeregrineAI route state
// ----------------------------------------------------
typedef struct {
    PStepList steps;                 // Path returned by Peregrine A*
    int index;                       // Current step index
    struct map_session_data* sd;     // Player session
    bool active;                     // Is routing active?
    GatMap* gmap;                    // Loaded GAT map (for walkability / reroute)
} PeregrineAI;

// Global route state
extern PeregrineAI g_route;

// ----------------------------------------------------
// API
// ----------------------------------------------------

// Start a new route
void pgn_route_start(const PluginContext* ctx,
                     struct map_session_data* sd,
                     PStepList* steps,
                     GatMap* gmap);

// Stop the current route
void pgn_route_stop(void);

// Check if a route is active
bool pgn_route_active(void);

#ifdef __cplusplus
}
#endif
