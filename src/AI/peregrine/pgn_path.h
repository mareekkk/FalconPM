#pragma once
#include <stdbool.h>
#include "pgn_gat.h"

typedef struct {
    int x, y;
} PStep;

struct PStepList {
    PStep* steps;
    int    count;
    int    capacity;
};

bool path_astar(const GatMap* g, int sx, int sy, int tx, int ty, struct PStepList* out);

typedef struct PStepList PStepList;
