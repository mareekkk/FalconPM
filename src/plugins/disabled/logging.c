/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: logging.c
 *  Description: FalconPM plugin — Auto-Logging (track kills, potions, damage, playtime; @autolog command)
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
#include "map/clif.h"
#include "map/battle.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));

/// Per-player logging data
struct log_data {
    int kills;
    int pots_used;
    int damage_done;
    unsigned int start_tick;
};

/// Attach log data when player enters
static void attach_log(struct map_session_data *sd) {
    if (!sd) return;
    if (!sd->reg) return;
    // Use script account regs for persistence
    if (pc_readaccountreg(sd, script->add_str("#log_start")) == 0) {
        pc_setaccountreg(sd, script->add_str("#log_kills"), 0);
        pc_setaccountreg(sd, script->add_str("#log_pots"), 0);
        pc_setaccountreg(sd, script->add_str("#log_dmg"), 0);
        pc_setaccountreg(sd, script->add_str("#log_start"), gettick());
    }
}

/// Count kill event
static void on_kill(struct map_session_data *sd, struct block_list *bl) {
    int kills = pc_readaccountreg(sd, script->add_str("#log_kills"));
    pc_setaccountreg(sd, script->add_str("#log_kills"), kills + 1);
}

/// Count potion use (called by autopots)
void log_potion_use(struct map_session_data *sd) {
    int pots = pc_readaccountreg(sd, script->add_str("#log_pots"));
    pc_setaccountreg(sd, script->add_str("#log_pots"), pots + 1);
}

/// Count damage dealt
static void on_damage(struct map_session_data *sd, int dmg) {
    int total = pc_readaccountreg(sd, script->add_str("#log_dmg"));
    pc_setaccountreg(sd, script->add_str("#log_dmg"), total + dmg);
}

/// Player command: @autolog
ACMD_FUNC(autolog) {
    int kills = pc_readaccountreg(sd, script->add_str("#log_kills"));
    int pots  = pc_readaccountreg(sd, script->add_str("#log_pots"));
    int dmg   = pc_readaccountreg(sd, script->add_str("#log_dmg"));
    unsigned int start = pc_readaccountreg(sd, script->add_str("#log_start"));
    unsigned int mins  = (gettick() - start) / (1000*60);

    clif->message(fd, "[FalconPM] ===== Logging Stats =====");
    char msg[128];
    sprintf(msg, "Kills: %d | Potions used: %d | Damage: %d", kills, pots, dmg);
    clif->message(fd, msg);
    sprintf(msg, "Playtime: %u min", mins);
    clif->message(fd, msg);
    return true;
}

HPExport void plugin_init(void) {
    addAtcommand("autolog", autolog);

    // Register events
    // Hook kill event
    addHookPost(battle, dead, on_kill);
    // Hook damage event (player deals damage)
    addHookPost(battle, damage, on_damage);
}
