#include "map/pc.h"
#include "map/script.h"
#include "map/status.h"
#include "map/itemdb.h"
#include "map/clif.h"

extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
extern void falconpm_delay(int,int);
extern bool falconpm_skip(int);

#define MAX_STORAGE_FILTER 5

static void autostorage_tick(struct map_session_data *sd) {
    if (!pc_readaccountreg(sd, script->add_str("#auto_storage_enabled"))) return;

    int limit = pc_readaccountreg(sd, script->add_str("#auto_storage_weight"));
    if (limit <= 0) limit = 80;

    int max_weight = sd->max_weight;
    int cur_weight = sd->weight;
    if (cur_weight * 100 / max_weight < limit) return;

    int return_mode = pc_readaccountreg(sd, script->add_str("#auto_storage_return")); // 0=walk,1=Butterfly Wing
    if (return_mode == 1) {
        int idx = pc_search_inventory(sd, ITEMID_BWING);
        if (idx >= 0) {
            falconpm_delay(200, 400);
            pc->useitem(sd, idx);
            clif->message(sd->fd, "[FalconPM] Butterfly Wing used. Returning to savepoint.");
            return;
        }
    }

    clif->message(sd->fd, "[FalconPM] Overweight: storage routine triggered.");
    // (Storage logic continues here)
}

HPExport void plugin_init(void) {
    falconpm_register("autostorage", autostorage_tick);
}
