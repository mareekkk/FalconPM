#include "mln_attack.h"
#include <stdio.h>

// TODO: use Peregrine to move into range, then call rAthena attack
bool mln_attack_execute(map_session_data* sd, MobTarget* t) {
    if (!t || !t->valid) return false;

    printf("[merlin] Attacking mob %d at (%d,%d)\n", t->mob_id, t->x, t->y);

    // Later: call ctx->peregrine->astar to move into range
    // Then: ctx->unit->attack(sd, t->mob_id)

    return true;
}
