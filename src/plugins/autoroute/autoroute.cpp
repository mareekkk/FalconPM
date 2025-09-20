#include "../infra/plugin_api.h"
#include <cstdio>
#include <cstdlib>

// Local state
static const PlayerAPI* pc   = nullptr;
static const UnitAPI*   unit = nullptr;

// Tick function — simulate walking
static void autoroute_tick(map_session_data* sd) {
    if (!sd) return;

    // Pick random destination
    int x = rand() % 100;
    int y = rand() % 100;

    // Future: real movement once UnitAPI exposes walktoxy
    fprintf(stdout, "[autoroute] would walk to (%d,%d)\n", x, y);
}

// Atcommand handler (future expansion)
// For now just call autoroute_tick and stub player name
static int atcommand_ar(map_session_data* sd, const char* command, const char* message) {
    (void)command; (void)message;
    const char* name = "(unknown)"; // TODO: expose via PlayerAPI
    fprintf(stdout, "[autoroute] @ar used by %s\n", name);
    autoroute_tick(sd);
    return 0;
}

// Required deps
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

    // TODO: add real ctx->atcommand once AtcommandAPI exists
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
