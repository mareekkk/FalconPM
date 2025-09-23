#include "mln_target.h"
#include <stdio.h>

// Dummy implementation: always return 1 mob
int mln_target_list(MobTarget* out, int max_count) {
    if (max_count < 1) return 0;

    out[0].id = 1;
    out[0].x = 100;
    out[0].y = 200;

    printf("[Merlin] Target acquired: id=%d at (%d,%d)\n", out[0].id, out[0].x, out[0].y);
    return 1;
}

MobTarget* mln_target_find(void) {
    // This function is not used by AutoAttack integration
    // AutoAttack finds its own targets and passes them to Merlin
    printf("[Merlin] mln_target_find called (not used in AutoAttack mode)\n");
    return NULL;
}

void mln_target_free(MobTarget* arr) {
    // no-op for now
}
