#pragma once
#include <stdbool.h>
#include "peregrine_gat.h"
#include "peregrine_path.h"

// Forward declare rAthena session
struct map_session_data;

// High-level OpenKore-style auto-walker
// mapname: map the player should be on (e.g. "gef_fild07")
// tx, ty: target coordinates
// Returns true if path execution started
bool peregrineAI(struct map_session_data* sd, const char* mapname, int tx, int ty);
