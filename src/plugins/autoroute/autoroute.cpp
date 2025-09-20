// /home/marek/FalconPM/src/plugins/autoroute/autoroute.cpp
//
// FalconPM Autoroute plugin
// Provides @ar <x> <y> command to walk the player to a target cell.
//

#include "../../infra/plugin_api.h"   // FalconPM API
#include "map/mmo.hpp"                // struct map_session_data
#include "map/clif.hpp"               // clif_displaymessage
#include "map/unit.hpp"               // unit_walktoxy
#include "map/block.hpp"              // struct block_list
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ----------------------------------------------------
// Command handler for @ar
// ----------------------------------------------------
static int at_ar(map_session_data* sd, const char* cmd, const char* args) {
    if (!sd) return -1;

    int x, y;
    if (sscanf(args, "%d %d", &x, &y) < 2) {
        clif_displaymessage(sd->fd, "Usage: @ar <x> <y>");
        return -1;
    }

    // Logging
    fprintf(stdout, "[autoroute] @ar used\n");
    fprintf(stdout, "[autoroute] walking to (%d,%d)\n", x, y);

    // Issue movement command
    unit_walktoxy(&sd->bl, (short)x, (short)y, 0, 0);

    return 0;
}

// ----------------------------------------------------
// FalconPM plugin boilerplate
// ----------------------------------------------------
static bool init(const PluginContext* ctx) {
    if (!ctx || !ctx->atcommand) {
        fprintf(stderr, "[autoroute] missing AtcommandAPI\n");
        return false;
    }

    // Register @ar
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
    "0.3",
    nullptr,   // required modules
    init,
    shutdown
};
}
