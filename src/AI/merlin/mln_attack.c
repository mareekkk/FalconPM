#include "mln_attack.h"
#include <stdbool.h>
#include <stdio.h>

// Internal state
static bool attack_active = false;

bool mln_attack_start(void* mob) {
    // TODO: implement attack logic here
    // For now, just stab it
    MobTarget* t = (MobTarget*)mob;
    if (!t) return false;
    printf("[Merlin] Starting attack on mob id=%d at (%d,%d)\n", t->id, t->x, t->y);
    attack_active = true;
    return true;
}

bool mln_attack_in_progress(void) {
    // Stubbed: in real shim, query PlayerAPI/UnitAPI to see if still attacking
    return attack_active;
}

bool mln_attack_done(void) {
    if (attack_active) {
        printf("[Merlin] Attack finished.\n");
        attack_active = false;
        return true;
    }
    return false;
}
