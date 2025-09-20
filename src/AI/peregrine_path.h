#pragma once
#include <stdbool.h>
#include "peregrine_gat.h"

typedef struct {
    int x, y;
} PStep;

typedef struct {
    PStep* steps;
    int count;
    int capacity;
} PStepList;

bool path_astar(const GatMap* g, int sx, int sy, int tx, int ty, PStepList* out);
