#include "plugin_api.h"
#include <cstdio>
#include <cstdlib>

// Local state
static const PlayerAPI* pc = nullptr;
static const UnitAPI*   unit = nullptr;

// Example tick function
static void autoroute_tick(map_session_data* sd) {
    if (!sd) return;

    // Pick random destination
    int x = rand() % 100;
    int y = rand() % 100;

    // Future: move player with UnitAPI
    // unit->walktoxy(sd->bl, x, y, 0);
    fprintf(stdout, "[autoroute] walking to (%d,%d)\n", x, y);
}

// Declare required modules
static const FpmModuleId* required_modules(size_t* count) {
    static const FpmModuleId deps[] = { FPM_MOD_PLAYER, FPM_MOD_UNIT };
    *count = sizeof(deps) / sizeof(deps[0]);
    return deps;
}

// Init
static bool init(const PluginContext* ctx) {
    pc   = ctx->player;
    unit = ctx->unit;
    if (!pc || !unit) return false;

    fprintf(stdout, "[autoroute] init OK\n");
    return true;
}

static void shutdown(void) {
    fprintf(stdout, "[autoroute] shutdown\n");
}

// Export descriptor (⚠️ no extra extern keyword!)
extern "C" {
PluginDescriptor PLUGIN = {
    "autoroute",
    "0.1",
    required_modules,
    init,
    shutdown
};
}
