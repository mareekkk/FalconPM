#include "mln_api.h"
#include "merlin.h"

// Forwarded to merlin.cpp functions
MerlinAPI merlin_api = {
    {1,0},
    merlin_tick,
    merlin_set_hunt_item,
    merlin_clear_plan
};
