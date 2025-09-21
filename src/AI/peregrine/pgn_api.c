#include <stdlib.h>
#include "pgn_api.h"

// ----------------------------------------------------
// Local helper to free step lists
// ----------------------------------------------------
static void peregrine_free_steps(PStepList* l) {
    if (!l) return;
    free(l->steps);
    l->steps = NULL;
    l->count = 0;
    l->capacity = 0;
}

// ----------------------------------------------------
// Expose Peregrine PeregrineAPI
// ----------------------------------------------------
PeregrineAPI peregrine_api = {
    .load_gat    = gat_load,
    .free_gat    = gat_free,
    .is_walkable = gat_is_walkable,
    .astar       = path_astar,
    .free_steps  = peregrine_free_steps
};
