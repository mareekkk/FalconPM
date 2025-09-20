// /home/marek/FalconPM/src/plugins/autoroute/autoroute.cpp
//
// FalconPM Autoroute plugin
// Provides @ar <x> <y> command to walk the player to a target cell.
//

#include "../../infra/plugin_api.h"   // FalconPM API
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ----------------------------------------------------
// Access FalconPM context from base
// ----------------------------------------------------
extern "C" const PluginContext* falconpm_get_context(void);
static const PluginContext* ctx = nullptr;

// ----------------------------------------------------
// Command handler for @ar
// ----------------------------------------------------
static int at_ar(map_session_data* sd, const char* cmd, const char* args) {
    if (!sd) return -1;

    int x, y;
    if (sscanf(args, "%d %d", &x, &y) < 2) {
        if (ctx && ctx->player && ctx->player->send_message)
            ctx->player->send_message(0, "Usage: @ar <x> <y>");
        return -1;
    }

    ctx->log->info("[autoroute] @ar used");
    ctx->log->info("[autoroute] walking to (%d,%d)", x, y);

    if (ctx->movement && ctx->movement->pc_walktoxy) {
        ctx->movement->pc_walktoxy(sd, (short)x, (short)y, 0);
    } else {
        ctx->log->error("[autoroute] movement API not available!");
    }

    return 0;
}

// ----------------------------------------------------
// FalconPM plugin boilerplate
// ----------------------------------------------------
static bool init(const PluginContext* c) {
    ctx = falconpm_get_context();
    if (!ctx || !ctx->atcommand) {
        fprintf(stderr, "[autoroute] missing AtcommandAPI\n");
        return false;
    }

    ctx->atcommand->add("ar", at_ar);
    fprintf(stdout, "[autoroute] init OK\n");
    return true;
}

static void shutdown(void) {
    fprintf(stdout, "[autoroute] shutdown\n");
}

extern "C" {
PluginDescriptor PLUGIN = {
    "autoroute",
    "0.5",
    nullptr,   // required modules
    init,
    shutdown
};
}
