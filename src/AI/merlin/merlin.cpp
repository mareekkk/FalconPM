#include "merlin.h"
#include "mln_target.h"
#include "mln_attack.h"

static bool merlin_enabled = true;   // controlled by plugin
static int hunt_item_id = 0;         // from NPC

bool merlin_tick(map_session_data* sd) {
    if (!merlin_enabled || !sd) return false;

    // ask target module for next monster
    MobTarget t = mln_target_find(sd, hunt_item_id);
    if (!t.valid) return false;

    // execute attack module
    return mln_attack_execute(sd, &t);
}

void merlin_set_hunt_item(int item_id) {
    hunt_item_id = item_id;
}

void merlin_clear_plan(void) {
    hunt_item_id = 0;
}
