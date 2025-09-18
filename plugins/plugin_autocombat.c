#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/unit.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);

#define MAX_ROTATION 5

// ------------------------------------------------------------
// Helper: Find nearest monster in range
// ------------------------------------------------------------
static struct block_list* find_nearest_mob(struct map_session_data *sd, int range) {
    struct block_list *bl, *target = NULL;
    int best_dist = range + 1;
    for (bl = map->list[sd->bl.m].block.mobs; bl; bl = bl->next) {
        struct mob_data *md = BL_CAST(BL_MOB, bl);
        if (!md || md->status.hp <= 0) continue;
        int dist = distance_bl(&sd->bl, &md->bl);
        if (dist < best_dist) { best_dist = dist; target = &md->bl; }
    }
    return target;
}

// ------------------------------------------------------------
// Auto-combat main loop
// ------------------------------------------------------------
static void autocombat_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str("#auto_combat_enabled"));
    int range   = pc_readaccountreg(sd, script->add_str("#auto_combat_range"));
    if (!enabled) return;
    if (range <= 0) range = 9;

    // Context awareness: skip if dead, sitting, or casting
    if (pc_issit(sd) || pc_isdead(sd)) return;
    if (sd->skilltimer != INVALID_TIMER) return;

    // Humanization: sometimes idle (2% chance)
    if (falconpm_skip(2)) return;

    // Humanization: random reaction time
    falconpm_delay(250, 700);

    struct block_list *target = find_nearest_mob(sd, range);
    if (!target) return;

    // --------------------------------------------------------
    // Action priority shuffle (randomize order occasionally)
    // --------------------------------------------------------
    int order[MAX_ROTATION];
    for (int i = 0; i < MAX_ROTATION; i++) order[i] = i+1;
    if (falconpm_skip(20)) { // 20% chance to shuffle
        for (int i = MAX_ROTATION-1; i > 0; i--) {
            int j = rand() % (i+1);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }
    }

    // Try skills in rotation
    for (int k = 0; k < MAX_ROTATION; k++) {
        int i = order[k];
        char key[32];
        sprintf(key, "#auto_rot%d_id", i);
        int skill_id = pc_readaccountreg(sd, script->add_str(key));
        sprintf(key, "#auto_rot%d_lv", i);
        int skill_lv = pc_readaccountreg(sd, script->add_str(key));
        if (skill_id <= 0 || skill_lv <= 0) continue;
        if (pc_checkskill(sd, skill_id) < skill_lv) continue;
        if (status->sp_required(sd, skill_id, skill_lv) > sd->status.sp) continue;

        skill_castend_id(sd, skill_id, skill_lv, target->id, gettick(), 0);
        clif->message(sd->fd, "[FalconPM] Rotation skill used.");
        return;
    }

    // Fallback: normal attack
    unit->attack(&sd->bl, target->id, 1);
    clif->message(sd->fd, "[FalconPM] Normal attack.");
}

HPExport void plugin_init(void) {
    falconpm_register("autocombat", autocombat_tick);
}
