// --------------------------------------------------------------------
// Plugin: AutoAttack
// --------------------------------------------------------------------
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstring>

// Global toggle
static bool g_enabled = false;

// Forward declaration
// [PATCH] Must return int, not void
static int aa_tick(int tid, unsigned long tick, int id, long data);

// --------------------------------------------------------------------
// Command handler (AtCmdFunc signature)
// AtCmdFunc in plugin_api.h is: int (*)(map_session_data*, const char*, const char*)
// --------------------------------------------------------------------
static int cmd_aa(struct map_session_data* sd,
                  const char* command, const char* message) {
    const PluginContext* ctx = falconpm_get_context();
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

        // Merlin stops automatically because g_enabled=false

    } else {
        ctx->log->info("[AutoAttack] Invalid arg. Usage: @aa <on|off>");
    }

    return 0;
}

// --------------------------------------------------------------------
// Tick handler (FpmTimerFunc signature)
// FpmTimerFunc in plugin_api.h is: int (*)(int, unsigned long, int, long)
// --------------------------------------------------------------------
static int aa_tick(int tid, unsigned long tick, int id, long data) {
    const PluginContext* ctx = falconpm_get_context();
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
// Init
// --------------------------------------------------------------------
extern "C" {
    bool autoattack_init() {
        auto ctx = falconpm_get_context();
        if (!ctx || !ctx->atcommand) return false;

        // Register @aa command
        ctx->atcommand->add("aa", cmd_aa);

        return true;
    }
}
