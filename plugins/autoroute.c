// ============================================================
// autoroute.c
// FalconPM plugin: Auto-Route system (@ar command)
// Uses fpm_graph + fpm_path + fpm_route
// ============================================================

#include "map/pc.h"
#include "map/script.h"
#include "map/clif.h"
#include "map/map.h"
#include "map/npc.h"

extern void fpm_portals_load(const char *filename);
extern void fpm_graph_add_edge(const char *src, const char *dst);
extern bool fpm_route_plan(const char *start, const char *goal, void *out);
extern void fpm_route_execute(const char *start_map, int start_x, int start_y,
                              const char *goal_map, int goal_x, int goal_y);

// ------------------------------------------------------------
// Account variables for autoroute
// ------------------------------------------------------------
static const char *VAR_ENABLED = "#autoroute_enabled";
static const char *VAR_TARGET_MAP = "#autoroute_target_map";
static const char *VAR_TARGET_X   = "#autoroute_target_x";
static const char *VAR_TARGET_Y   = "#autoroute_target_y";

// ------------------------------------------------------------
// Tick loop (called every 500ms by FalconPM base)
// ------------------------------------------------------------
static void autoroute_tick(struct map_session_data *sd) {
    int enabled = pc_readaccountreg(sd, script->add_str(VAR_ENABLED));
    if (!enabled) return;

    const char *target_map = script->strdup(pc_readregistry(sd, script->add_str(VAR_TARGET_MAP)));
    int tx = pc_readaccountreg(sd, script->add_str(VAR_TARGET_X));
    int ty = pc_readaccountreg(sd, script->add_str(VAR_TARGET_Y));

    if (!target_map || tx <= 0 || ty <= 0) {
        clif->message(sd->fd, "[FPM] Autoroute: No valid target set.");
        return;
    }

    // Current map + pos
    const char *cur_map = mapindex_id2name(sd->mapindex);
    int x = sd->bl.x;
    int y = sd->bl.y;

    // Call executor
    fpm_route_execute(cur_map, x, y, target_map, tx, ty);
}

// ------------------------------------------------------------
// @ar command handler
// ------------------------------------------------------------
static int atcommand_ar(const int fd, struct map_session_data *sd,
                        const char *command, const char *message) {
    if (!message || strlen(message) == 0) {
        clif->message(fd, "Usage: @ar <map> <x> <y>");
        return 0;
    }

    char map[64];
    int x, y;
    if (sscanf(message, "%63s %d %d", map, &x, &y) != 3) {
        clif->message(fd, "Usage: @ar <map> <x> <y>");
        return 0;
    }

    pc_setaccountregstr(sd, script->add_str(VAR_TARGET_MAP), map);
    pc_setaccountreg(sd, script->add_str(VAR_TARGET_X), x);
    pc_setaccountreg(sd, script->add_str(VAR_TARGET_Y), y);
    pc_setaccountreg(sd, script->add_str(VAR_ENABLED), 1);

    char buf[128];
    sprintf(buf, "[FPM] Autoroute set: %s (%d,%d)", map, x, y);
    clif->message(fd, buf);

    return 0;
}

// ------------------------------------------------------------
// Plugin init
// ------------------------------------------------------------
HPExport void plugin_init(void) {
    atcommand->add("ar", atcommand_ar);
    // Register tick with FalconPM base
    extern void falconpm_register(const char *name, void (*cb)(struct map_session_data*));
    falconpm_register("autoroute", autoroute_tick);

    ShowInfo("[FPM] Auto-Route plugin loaded (@ar)\n");
}
