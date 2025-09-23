#pragma once
#include "../infra/plugin_api.h"
#include <stdbool.h>
#include "pgn_gat.h"
#include "pgn_path.h"

// Global PeregrineAPI table implemented in peregrine_ai.c
extern PeregrineAPI peregrine_api;

// Forward declare rAthena session
struct map_session_data;

// New helper to let AI check if the character is still following a route
bool route_is_busy(struct map_session_data* sd);
bool pgn_route_active(void);  // already exists in peregrine.cpp