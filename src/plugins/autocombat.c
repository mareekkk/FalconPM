/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: autocombat.c
 *  Description: FalconPM plugin — Auto-Combat (battle automation, teleport escape, mob filtering)
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  Licensed under GNU General Public License v3 or later.
 *  See <https://www.gnu.org/licenses/>.
 */

 #include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/skill.h"
#include "map/unit.h"
#include "map/clif.h"
#include "map/itemdb.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);

#define MAX_ROTATION 5
#define MAX_FILTER   5

// ------------------------------------------------------------
// Helper: check mob filter
// ------------------------------------------------------------
static bool mob_allowed(int mob_id, struct map_session_data *sd) {
    int has_filter = 0;
    for (int i = 1; i <= MAX_FILTER; i++) {
        char key[32];
        sprintf(key, "#auto_combat_mob%d", i);
        int f = pc_readaccountreg(sd, script->add_str(key));
        if (f > 0) {
            has_filter = 1;
            if (f == mob_id) return true;
        }
    }
    return has_filter ? false : true;
}

// ------------------------------------------------------------
// Count mobs around
// ------------------------------------------------------------
static int mob_count_nearby(struct map_session_data *sd, int range) {
    int count = 0;
    struct block_list *bl;
    for (bl = map->list[sd->bl.m].block.mobs; bl; bl = bl->next) {
        struct mob_data *md = BL_CAST(BL_MOB, bl);
        if (!md || md->status.hp <= 0) continue;
        if (!mob_allowed(md->class_, sd)) continue;
        if (distance_bl(&sd->bl, &md->bl) <= range) count++;
    }
    return count;
}

// ------------------------------------------------------------
// Try teleport (skill or Fly Wing)
// ------------------------------------------------------------
static bool try_teleport(struct map_session_data *sd, int mode) {
    if (mode == 1) { // Teleport skill
        if (pc_checkskill(sd, AL_TELEPORT) > 0) {
            falconpm_delay(200, 400);
            skill_castend_nodamage_id(&sd->bl, &sd->bl, AL_TELEPORT, 1, gettick(), 0);
            clif->message(sd->fd, "[FalconPM] Teleport skill used.");
            return true;
        }
    }
    if (mode == 2) { // Fly Wing item
        int idx = pc_search_inventory(sd, ITEMID_FLYWING);
        if (idx >= 0) {
            falconpm_delay(200, 400);
            pc->useitem(sd, idx);
            clif->message(sd->fd, "[FalconPM] Fly Wing used.");
            return true;
        }
    }
    return false;
}

// ------------------------------------------------------------
// Auto-combat tick
// ------------------------------------------------------------
static void autocombat_tick(struct map_session_data *sd) {
    if (!pc_readaccountreg(sd, script->add_str("#auto_combat_enabled"))) return;

    if (pc_isdead(sd) || pc_issit(sd)) return;

    // --- Teleport escape check ---
    int tp_mode = pc_readaccountreg(sd, script->add_str("#auto_combat_teleport")); // 0=off,1=skill,2=fly wing
    int hp_th   = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_hp"));
    int mob_th  = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_mob"));
    int boss_on = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_boss"));

    if (tp_mode > 0) {
        int hp_pct = (sd->status.hp * 100) / sd->status.max_hp;
        if (hp_th > 0 && hp_pct < hp_th) {
            if (try_teleport(sd, tp_mode)) return;
        }
        if (mob_th > 0 && mob_count_nearby(sd, 9) >= mob_th) {
            if (try_teleport(sd, tp_mode)) return;
        }
        if (boss_on) {
            struct block_list *bl;
            for (bl = map->list[sd->bl.m].block.mobs; bl; bl = bl->next) {
                struct mob_data *md = BL_CAST(BL_MOB, bl);
                if (!md) continue;
                if (mobdb_isboss(md->class_)) {
                    if (try_teleport(sd, tp_mode)) return;
                }
            }
        }
    }

    // (existing combat logic continues here… skill rotation, attack, etc.)
}

HPExport void plugin_init(void) {
    falconpm_register("autocombat", autocombat_tick);
}
