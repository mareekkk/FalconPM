#include "map/pc.h"
#include "map/script.h"
#include "map/status.h"
#include "map/itemdb.h"
#include "map/npc.h"
#include "map/clif.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);

#define MAX_STORAGE_FILTER 5

// ------------------------------------------------------------
// Helper: check if item should be stored
// ------------------------------------------------------------
static bool store_allowed(int nameid, struct map_session_data *sd) {
    int mode = pc_readaccountreg(sd, script->add_str("#auto_storage_filter_mode"));
    int has_filter = 0;

    for (int i = 1; i <= MAX_STORAGE_FILTER; i++) {
        char key[32];
        sprintf(key, "#auto_storage_item%d", i);
        int f = pc_readaccountreg(sd, script->add_str(key));
        if (f <= 0) continue;

        has_filter = 1;
        if (f == nameid) {
            sprintf(key, "#auto_storage_max%d", i);
            int keep = pc_readaccountreg(sd, script->add_str(key));
            int count = pc_countitem(sd, nameid);
            if (count > keep) {
                if (mode == 1) return true;   // whitelist → store excess
                if (mode == 2) return false;  // blacklist → don't store
            } else {
                return false;
            }
        }
    }

    if (mode == 1 && has_filter) return false; // whitelist mode → deny if not listed
    if (mode == 2 && has_filter) return true;  // blacklist mode → store if not listed
    return true; // mode=0 or no filter → store all
}

// ------------------------------------------------------------
// Auto-Storage tick
// ------------------------------------------------------------
static void autostorage_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str("#auto_storage_enabled"));
    if (!enabled) return;

    int limit = pc_readaccountreg(sd, script->add_str("#auto_storage_weight"));
    if (limit <= 0) limit = 80; // default trigger

    int max_weight = sd->max_weight;
    int cur_weight = sd->weight;
    if (cur_weight * 100 / max_weight < limit) return; // not overweight yet

    // Humanization
    if (falconpm_skip(10)) return;
    falconpm_delay(500, 1000);

    // TODO: For now, just send message (moving to Kafra would need route logic)
    clif->message(sd->fd, "[FalconPM] Auto-Storage triggered (overweight).");

    // Store items
    for (int i = 0; i < sd->inventory.size; i++) {
        if (sd->status.inventory[i].nameid > 0 &&
            store_allowed(sd->status.inventory[i].nameid, sd)) {
            int amount = sd->status.inventory[i].amount;
            pc->storage_additem(sd, &sd->status.inventory[i], amount);
            clif->message(sd->fd, "[FalconPM] Stored item.");
        }
    }
}

HPExport void plugin_init(void) {
    falconpm_register("autostorage", autostorage_tick);
}
