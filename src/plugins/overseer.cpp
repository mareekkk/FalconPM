// --------------------------------------------------------------------
// Plugin: Overseer
// Role: Top-level orchestrator controller (delegates to Hunter)
// --------------------------------------------------------------------
#include "../infra/plugin_api.h"
#include "../core/falconpm.hpp"
#include <cstring>

static const PluginContext* ctx = nullptr;

// --------------------------------------------------------------------
// Command handler (AtCmdFunc signature)
// --------------------------------------------------------------------
static int cmd_overseer(struct map_session_data* sd,
                        const char* command, const char* message) {
    if (!ctx || !ctx->log) return 0;

    if (!message || !*message) {
        ctx->log->info("[Overseer] Usage: @ov <on|off>");
        return 0;
    }

    if (!strcmp(message, "on")) {
        ctx->log->info("[Overseer] Enabled — delegating to Hunter");

        if (ctx->hunter) {
            ctx->hunter->enable_autoattack(sd);
        } else {
            ctx->log->error("[Overseer] Hunter API not available");
        }

    } else if (!strcmp(message, "off")) {
        ctx->log->info("[Overseer] Disabled — delegating to Hunter");

        if (ctx->hunter) {
            ctx->hunter->disable_autoattack();
        } else {
            ctx->log->error("[Overseer] Hunter API not available");
        }

    } else {
        ctx->log->info("[Overseer] Invalid arg. Usage: @ov <on|off>");
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

    // Register @ov command
    ctx->atcommand->add("ov", cmd_overseer);

    if (ctx->log) {
        ctx->log->info("[Overseer] Plugin loaded - @ov command available");
    }
    return true;
}

void plugin_final() {
    ctx = nullptr;
}

PluginDescriptor PLUGIN = {
    "overseer",
    "1.0",
    nullptr,
    plugin_init,
    plugin_final
};

} // extern "C"
