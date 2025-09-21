#pragma once
#include "../infra/plugin_api.h"
#include <stdbool.h>
#include "peregrine_gat.h"
#include "peregrine_path.h"

// Global SmartAPI table implemented in peregrine_ai.c
extern SmartAPI smart_api;

// Forward declare rAthena session
struct map_session_data;

bool peregrineAI(struct map_session_data* sd, const char* mapname, int tx, int ty);
