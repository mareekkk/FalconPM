// --------------------------------------------------------------------
// Plugin: AutoAttack
// --------------------------------------------------------------------
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstring>

// Global toggle
static bool g_enabled = false;
static const PluginContext* ctx = nullptr;

// Forward declaration
static int aa_tick(int tid, unsigned long tick, int id, long data);

// --------------------------------------------------------------------
// Command handler (AtCmdFunc signature)
// --------------------------------------------------------------------
static int cmd_aa(struct map_session_data* sd,
                  const char* command, const char* message) {
    if (!ctx || !ctx->log) return 0;

    if (!message || !*message) {
        ctx->log->info("[AutoAttack] Usage: @aa <on|off>");
        return 0;
    }

    if (!strcmp(message, "on")) {
        if (g_enabled) {
            ctx->log->info("[AutoAttack] Already enabled");
            return 0;
        }

        g_enabled = true;
        ctx->log->info("[AutoAttack] Combat enabled - Merlin taking control");

        // [PATCH] Start Lanner when AutoAttack is enabled
        if (ctx->lanner) {
            ctx->lanner->start(sd);
            ctx->log->info("[AutoAttack] Lanner buff system started");
        }

        // Start Merlin attack loop
        if (ctx->timer) {
            ctx->timer->add_timer(ctx->timer->gettick() + 100, aa_tick, 0, 0);
        }

    } else if (!strcmp(message, "off")) {
        if (!g_enabled) {
            ctx->log->info("[AutoAttack] Already disabled");
            return 0;
        }

        g_enabled = false;
        ctx->log->info("[AutoAttack] Combat disabled");

        // [PATCH] Stop Lanner when AutoAttack is disabled
        if (ctx->lanner) {
            ctx->lanner->stop();
            ctx->log->info("[AutoAttack] Lanner buff system stopped");
        }

    } else {
        ctx->log->info("[AutoAttack] Invalid arg. Usage: @aa <on|off>");
    }

    return 0;
}

// --------------------------------------------------------------------
// Tick handler (FpmTimerFunc signature)
// --------------------------------------------------------------------
static int aa_tick(int tid, unsigned long tick, int id, long data) {
    if (!ctx || !g_enabled) return 0;

    // Run Merlin tick only when enabled
    if (ctx->merlin) {
        ctx->merlin->tick();
    }

    // Keep looping
    if (ctx->timer) {
        ctx->timer->add_timer(ctx->timer->gettick() + 100, aa_tick, 0, 0);
    }

    return 0;
}

// --------------------------------------------------------------------
// Plugin init/final (classic FalconPM contract)
// --------------------------------------------------------------------
extern "C" {

bool plugin_init(const PluginContext* u) {
    (void)u;
    ctx = falconpm_get_context();
    if (!ctx || !ctx->atcommand) return false;

    // Register @aa command
    ctx->atcommand->add("aa", cmd_aa);

    if (ctx->log) {
        ctx->log->info("[AutoAttack] Plugin loaded - @aa command available");
    }
    return true;
}

void plugin_final() {
    g_enabled = false;
    ctx = nullptr;
}

// Plugin descriptor
PluginDescriptor PLUGIN = {
    "autoattack",
    "1.0",
    nullptr,
    plugin_init,
    plugin_final
};

} // extern "C"
