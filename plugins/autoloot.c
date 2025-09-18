#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/unit.h"
#include "map/itemdb.h"
#include "map/clif.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);

#define MAX_LOOT_FILTER 5

// ------------------------------------------------------------
// Helper: check if item is allowed by filter
// ------------------------------------------------------------
static bool loot_allowed(int nameid, struct map_session_data *sd) {
    int mode = pc_readaccountreg(sd, script->add_str("#auto_loot_filter_mode"));
    int has_filter = 0;

    for (int i = 1; i <= MAX_LOOT_FILTER; i++) {
        char key[32];
        sprintf(key, "#auto_loot_item%d", i);
        int f = pc_readaccountreg(sd, script->add_str(key));
        if (f > 0) {
            has_filter = 1;
            if (f == nameid) {
                if (mode == 1) return true;   // whitelist → allowed
                if (mode == 2) return false;  // blacklist → denied
            }
        }
    }

    if (mode == 1 && has_filter) return false; // whitelist mode → deny if not in list
    if (mode == 2 && has_filter) return true;  // blacklist mode → allow if not in list
    return true; // mode=0 or no filter → allow all
}

// ------------------------------------------------------------
// Auto-loot tick
// ------------------------------------------------------------
static void autoloot_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str("#auto_loot_enabled"));
    if (!enabled) return;

    int range = pc_readaccountreg(sd, script->add_str("#auto_loot_range"));
    if (range <= 0) range = 3;

    if (pc_isdead(sd) || pc_issit(sd)) return;

    // Loop through nearby items
    struct block_list *bl;
    for (bl = map->list[sd->bl.m].block.items; bl; bl = bl->next) {
        struct flooritem_data *fid = BL_CAST(BL_ITEM, bl);
        if (!fid) continue;
        if (!loot_allowed(fid->item_data.nameid, sd)) continue;

        int dist = distance_bl(&sd->bl, &fid->bl);
        if (dist > range) continue;

        // Humanization
        if (falconpm_skip(5)) continue;  // 5% skip chance
        falconpm_delay(200, 600);

        // Attempt pickup
        pc->takeitem(sd, fid);
        const struct item_data *id = itemdb->search(fid->item_data.nameid);
        if (id)
            clif->message(sd->fd, "[FalconPM] Picked up " + (char*)id->jname);
        else
            clif->message(sd->fd, "[FalconPM] Picked up item.");
        return; // one item per tick
    }
}

HPExport void plugin_init(void) {
    falconpm_register("autoloot", autoloot_tick);
}
