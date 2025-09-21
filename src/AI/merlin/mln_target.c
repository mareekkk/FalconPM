#include "mln_target.h"
#include <stdio.h>

// TODO: replace with real monster DB lookup
MobTarget mln_target_find(map_session_data* sd, int hunt_item_id) {
    MobTarget t = {0};

    // For now, just pick the first mob in sight
    if (hunt_item_id > 0) {
        // Later: scan monster DB for drops matching hunt_item_id
        // and pick closest mob
        printf("[merlin] Hunting item %d (stub)\n", hunt_item_id);
    }

    // Stub: no target
    t.valid = false;
    return t;
}
