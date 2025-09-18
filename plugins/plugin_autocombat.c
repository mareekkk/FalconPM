#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/unit.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);
extern int falconpm_jitter(int,int);

#define MAX_ROTATION 5

// ------------------------------------------------------------
// Helper: Find nearest mob
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
// Auto-potion tick
// ------------------------------------------------------------
static void autopots_tick(struct map_session_data *sd) {
    int hp_th   = pc_readaccountreg(sd, script->add_str("#auto_hp_threshold"));
    int hp_item = pc_readaccountreg(sd, script->add_str("#auto_hp_item"));

    if (hp_th <= 0 || hp_item <= 0) return;
    if (pc_isdead(sd) || pc_issit(sd)) return;

    int hp_percent = sd->status.hp * 100 / sd->status.max_hp;
    hp_th = falconpm_jitter(hp_th, 3);

    if (hp_percent < hp_th) {
        if (falconpm_skip(5)) return;
        falconpm_delay(200, 600);

        pc_useitem(sd, hp_item);
        clif->message(sd->fd, "[FalconPM] Auto-potion used.");
    }
}

// ------------------------------------------------------------
// Auto-combat tick
// ------------------------------------------------------------
static void autocombat_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str("#auto_combat_enabled"));
    int range   = pc_readaccountreg(sd, script->add_str("#auto_combat_range"));
    if (!enabled) return;
    if (range <= 0) range = 9;

    if (pc_issit(sd) || pc_isdead(sd)) return;
    if (sd->skilltimer != INVALID_TIMER) return;
    if (falconpm_skip(2)) return;

    falconpm_delay(250, 700);

    struct block_list *target = find_nearest_mob(sd, range);
    if (!target) return;

    int order[MAX_ROTATION];
    for (int i = 0; i < MAX_ROTATION; i++) order[i] = i+1;
    if (falconpm_skip(20)) {
        for (int i = MAX_ROTATION-1; i > 0; i--) {
            int j = rand() % (i+1);
            int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }
    }

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

    unit->attack(&sd->bl, target->id, 1);
    clif->message(sd->fd, "[FalconPM] Normal attack.");
}

// ------------------------------------------------------------
// Combined Tick (Core loop)
// ------------------------------------------------------------
static void falconpm_core_tick(struct map_session_data *sd) {
    autopots_tick(sd);
    autocombat_tick(sd);
}

HPExport void plugin_init(void) {
    falconpm_register("autocore", falconpm_core_tick);
}
