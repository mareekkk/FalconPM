/**
 *  FalconPM - Random Walk Test Plugin
 *
 *  File: randomwalk.c
 *  Description: Makes the player randomly walk to positions on the same map.
 *
 *  Command: @rw <on|off>
 */

#include "map/pc.h"
#include "map/script.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/npc.h"
#include <stdlib.h>
#include <time.h>

static const char *VAR_ENABLED = "#randomwalk_enabled";

// ------------------------------------------------------------
// Tick loop (called every 500ms by FalconPM)
// ------------------------------------------------------------
static void randomwalk_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str(VAR_ENABLED));
    if (!enabled) return;

    // Current map info
    const char *cur_map = mapindex_id2name(sd->mapindex);
    int max_x = map->list[sd->bl.m].xs;
    int max_y = map->list[sd->bl.m].ys;

    // Pick a random nearby coordinate
    int rx = rand() % max_x;
    int ry = rand() % max_y;

    // Send a walk request to client
    clif->walktoxy(sd, rx, ry);

    char buf[64];
    sprintf(buf, "[FPM] RandomWalk moving to (%d,%d)", rx, ry);
    clif->message(sd->fd, buf);
}

// ------------------------------------------------------------
// @rw command handler
// ------------------------------------------------------------
static int atcommand_rw(const int fd, struct map_session_data *sd,
                        const char *command, const char *message) {
    if (!message || strlen(message) == 0) {
        clif->message(fd, "Usage: @rw <on|off>");
        return 0;
    }

    if (strcmp(message, "on") == 0) {
        pc_setaccountreg(sd, script->add_str(VAR_ENABLED), 1);
        clif->message(fd, "[FPM] RandomWalk enabled.");
    } else if (strcmp(message, "off") == 0) {
        pc_setaccountreg(sd, script->add_str(VAR_ENABLED), 0);
        clif->message(fd, "[FPM] RandomWalk disabled.");
    } else {
        clif->message(fd, "Usage: @rw <on|off>");
    }
    return 0;
}

// ------------------------------------------------------------
// Plugin init
// ------------------------------------------------------------
HPExport void plugin_init(PluginAPI* api) {
    srand(time(NULL));
    atcommand->add("rw", atcommand_rw);

    extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
    falconpm_register("randomwalk", randomwalk_tick);

    api->log_info("[FPM] RandomWalk plugin loaded (@rw)\n");
}
