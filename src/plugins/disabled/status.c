/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: status.c
 *  Description: FalconPM plugin — Auto-Status (@autostatus + short aliases for toggling modules)
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  Licensed under GNU General Public License v3 or later.
 *  See <https://www.gnu.org/licenses/>.
 */

#include "common/hercules.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/clif.h"
#include "map/atcommand.h"
#include <string.h>
#include <stdio.h>

// -------------------------------------------
// Helper: show dashboard
// -------------------------------------------
static void show_status(const int fd, struct map_session_data* sd) {
    clif->message(fd, "[FalconPM Status]");

    // Auto-Pots
    int pots_enabled = pc_readaccountreg(sd, script->add_str("#auto_pots_enabled"));
    int hp_th = pc_readaccountreg(sd, script->add_str("#auto_hp_threshold"));
    int hp_item = pc_readaccountreg(sd, script->add_str("#auto_hp_item"));
    if (pots_enabled)
        clif->message(fd, "Auto-Pots: Enabled (HP<%d%% / Item %d)", hp_th, hp_item);
    else
        clif->message(fd, "Auto-Pots: Disabled");

    // Auto-Combat
    int combat_enabled = pc_readaccountreg(sd, script->add_str("#auto_combat_enabled"));
    int tp_mode = pc_readaccountreg(sd, script->add_str("#auto_combat_teleport"));
    int tp_hp   = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_hp"));
    int tp_mob  = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_mob"));
    int tp_boss = pc_readaccountreg(sd, script->add_str("#auto_combat_tp_boss"));
    if (combat_enabled) {
        char tpinfo[128];
        if (tp_mode == 0) sprintf(tpinfo, "No teleport");
        else if (tp_mode == 1) sprintf(tpinfo, "Teleport Skill (HP<%d%%, Mob≥%d, Boss=%d)", tp_hp, tp_mob, tp_boss);
        else if (tp_mode == 2) sprintf(tpinfo, "Fly Wing (HP<%d%%, Mob≥%d, Boss=%d)", tp_hp, tp_mob, tp_boss);
        clif->message(fd, "Auto-Combat: Enabled [%s]", tpinfo);
    } else {
        clif->message(fd, "Auto-Combat: Disabled");
    }

    // Auto-Support
    int support_enabled = pc_readaccountreg(sd, script->add_str("#auto_support_enabled"));
    clif->message(fd, support_enabled ? "Auto-Support: Enabled" : "Auto-Support: Disabled");

    // Auto-Loot
    int loot_enabled = pc_readaccountreg(sd, script->add_str("#auto_loot_enabled"));
    int loot_limit   = pc_readaccountreg(sd, script->add_str("#auto_loot_weight_limit"));
    if (loot_limit <= 0) loot_limit = 90;
    if (loot_enabled)
        clif->message(fd, "Auto-Loot: Enabled (Weight <%d%%)", loot_limit);
    else
        clif->message(fd, "Auto-Loot: Disabled");

    // Auto-Storage
    int storage_enabled = pc_readaccountreg(sd, script->add_str("#auto_storage_enabled"));
    int storage_weight  = pc_readaccountreg(sd, script->add_str("#auto_storage_weight"));
    int storage_return  = pc_readaccountreg(sd, script->add_str("#auto_storage_return"));
    if (storage_weight <= 0) storage_weight = 80;
    if (storage_enabled) {
        clif->message(fd, "Auto-Storage: Enabled (Trigger>%d%%, Return=%s)", 
            storage_weight, storage_return == 1 ? "Butterfly Wing" : "Walk");
    } else {
        clif->message(fd, "Auto-Storage: Disabled");
    }
}

// -------------------------------------------
// Core toggle handler
// -------------------------------------------
static void toggle_module(struct map_session_data* sd, const char* module, int enable) {
    if (strcasecmp(module, "pots") == 0) {
        pc_setaccountreg(sd, script->add_str("#auto_pots_enabled"), enable);
        clif->message(sd->fd, enable ? "[FalconPM] Auto-Pots enabled." : "[FalconPM] Auto-Pots disabled.");
    } else if (strcasecmp(module, "combat") == 0) {
        pc_setaccountreg(sd, script->add_str("#auto_combat_enabled"), enable);
        clif->message(sd->fd, enable ? "[FalconPM] Auto-Combat enabled." : "[FalconPM] Auto-Combat disabled.");
    } else if (strcasecmp(module, "support") == 0 || strcasecmp(module, "sup") == 0) {
        pc_setaccountreg(sd, script->add_str("#auto_support_enabled"), enable);
        clif->message(sd->fd, enable ? "[FalconPM] Auto-Support enabled." : "[FalconPM] Auto-Support disabled.");
    } else if (strcasecmp(module, "loot") == 0) {
        pc_setaccountreg(sd, script->add_str("#auto_loot_enabled"), enable);
        clif->message(sd->fd, enable ? "[FalconPM] Auto-Loot enabled." : "[FalconPM] Auto-Loot disabled.");
    } else if (strcasecmp(module, "storage") == 0 || strcasecmp(module, "stor") == 0) {
        pc_setaccountreg(sd, script->add_str("#auto_storage_enabled"), enable);
        clif->message(sd->fd, enable ? "[FalconPM] Auto-Storage enabled." : "[FalconPM] Auto-Storage disabled.");
    } else {
        clif->message(sd->fd, "Unknown module. Options: pots, combat, support, loot, storage");
    }
}

// -------------------------------------------
// Main @autostatus command
// -------------------------------------------
static int atcommand_autostatus(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    if (message == NULL || strlen(message) == 0) {
        show_status(fd, sd);
        return 0;
    }

    char arg1[64], arg2[64];
    if (sscanf(message, "%63s %63s", arg1, arg2) < 2) {
        clif->message(fd, "Usage: @autostatus <module> <on|off>");
        return 0;
    }

    int enable = (strcasecmp(arg2, "on") == 0) ? 1 : 0;
    toggle_module(sd, arg1, enable);

    return 0;
}

// -------------------------------------------
// Short aliases (@ap, @ac, @asup, @al, @ast)
// -------------------------------------------
static int atcommand_ap(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    int enable = (message && strcasecmp(message, "on") == 0) ? 1 : 0;
    toggle_module(sd, "pots", enable);
    return 0;
}

static int atcommand_ac(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    int enable = (message && strcasecmp(message, "on") == 0) ? 1 : 0;
    toggle_module(sd, "combat", enable);
    return 0;
}

static int atcommand_asup(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    int enable = (message && strcasecmp(message, "on") == 0) ? 1 : 0;
    toggle_module(sd, "support", enable);
    return 0;
}

static int atcommand_al(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    int enable = (message && strcasecmp(message, "on") == 0) ? 1 : 0;
    toggle_module(sd, "loot", enable);
    return 0;
}

static int atcommand_ast(const int fd, struct map_session_data* sd, const char* command, const char* message) {
    int enable = (message && strcasecmp(message, "on") == 0) ? 1 : 0;
    toggle_module(sd, "storage", enable);
    return 0;
}

// -------------------------------------------
// Plugin init
// -------------------------------------------
HPExport void plugin_init(void) {
    atcommand->add("autostatus", atcommand_autostatus);
    atcommand->add("ap", atcommand_ap);
    atcommand->add("ac", atcommand_ac);
    atcommand->add("asup", atcommand_asup);
    atcommand->add("al", atcommand_al);
    atcommand->add("ast", atcommand_ast);
}
