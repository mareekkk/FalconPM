#include "common/hercules.h"
#include "map/pc.h"
#include "map/script.h"
#include "map/clif.h"
#include "map/atcommand.h"

static int atcommand_autostatus(const int fd, struct map_session_data* sd, const char* command, const char* message) {
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

    return 0;
}

HPExport void plugin_init(void) {
    atcommand->add("autostatus", atcommand_autostatus);
}
