#pragma once
#include "../infra/plugin_api.h"
#include <stdbool.h>
#include "pgn_gat.h"
#include "pgn_path.h"

// Global PeregrineAPI table implemented in peregrine_ai.c
extern PeregrineAPI peregrine_api;

// Forward declare rAthena session
struct map_session_data;

// test function bool peregrineAI(struct map_session_data* sd, const char* mapname, int tx, int ty);
