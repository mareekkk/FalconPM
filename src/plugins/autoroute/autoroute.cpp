#include "../../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>

// ----------------------------------------------------
// Local state
// ----------------------------------------------------
static const PlayerAPI*   pc   = nullptr;
static const UnitAPI*     unit = nullptr;
static const AtcommandAPI* atc = nullptr;

// ----------------------------------------------------
// Tick function — move randomly
// ----------------------------------------------------
static void autoroute_tick(map_session_data* sd) {
    if (!sd) return;

    int x = rand() % 100;
    int y = rand() % 100;

    // In the future, call unit->walktoxy()
    fprintf(stdout, "[autoroute] walking to (%d,%d)\n", x, y);
}

// ----------------------------------------------------
// Atcommand handler
// ----------------------------------------------------
static int atcommand_ar(map_session_data* sd, const char* command, const char* message) {
    (void)command; (void)message;
    fprintf(stdout, "[autoroute] @ar used\n");
    autoroute_tick(sd);
    return 0;
}

// ----------------------------------------------------
// Required modules
// ----------------------------------------------------
static const FpmModuleId* required_modules(size_t* count) {
    static const FpmModuleId deps[] = { FPM_MOD_PLAYER, FPM_MOD_UNIT, FPM_MOD_ATCOMMAND };
    *count = sizeof(deps)/sizeof(deps[0]);
    return deps;
}

// ----------------------------------------------------
// Init / Shutdown
// ----------------------------------------------------
static bool init(const PluginContext* ctx) {
    pc   = ctx->player;
    unit = ctx->unit;
    atc  = ctx->atcommand;
    if (!pc || !unit || !atc) return false;

    atc->add("ar", atcommand_ar);
    fprintf(stdout, "[autoroute] init OK\n");
    return true;
}

static void shutdown(void) {
    if (atc) atc->remove("ar");
    fprintf(stdout, "[autoroute] shutdown\n");
}

// ----------------------------------------------------
// Exported descriptor
// ----------------------------------------------------
extern "C" {
PluginDescriptor PLUGIN = {
    "autoroute",
    "0.1",
    required_modules,
    init,
    shutdown
};
}
