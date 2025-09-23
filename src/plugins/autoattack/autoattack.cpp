// src/plugins/autoattack/autoattack.cpp
// FalconPM AutoAttack Plugin - OpenKore Style (Combat Enabler Only)

#include "../../infra/plugin_api.h"
#include "../../AI/merlin/merlin.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstring>

static const PluginContext* ctx = nullptr;

// Reference core globals
extern int g_autoattack_account_id;      // Add this
extern GatMap* g_autoattack_map;         // Add this

static GatMap* load_gat(map_session_data* sd) {
    int m = fpm_get_sd_m(sd);
    const char* name = fpm_get_map_name(m);
    if (!name) return nullptr;
    char fn[512];
    snprintf(fn, sizeof(fn), FALCONPM_GAT_PATH "%s.gat", name);
    return ctx->peregrine->load_gat(fn);
}

static int cmd_aa(map_session_data* sd, const char* c, const char* m) {
    if (!sd) return -1;

    if (m && strcmp(m, "off") == 0) {
        // Disable combat
        // OLD (global): extern void mln_api_set_state(int state); mln_api_set_state(0);
        ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_IDLE); // NEW: API pointer
        ctx->log->info("[AutoAttack] Combat disabled");

        // Clean up
        g_autoattack_account_id = -1;
        if (g_autoattack_map) {
            ctx->peregrine->free_gat(g_autoattack_map);
            g_autoattack_map = nullptr;
        }
        return 0;
    }

    // Enable combat
    g_autoattack_account_id = ctx->player->get_account_id(sd);
    g_autoattack_map = load_gat(sd);

    if (!g_autoattack_map) {
        ctx->log->error("[AutoAttack] Failed to load map data");
        return -1;
    }

    // OLD (global): extern void mln_api_set_state(int state); mln_api_set_state(1);
    ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_ROAMING); // NEW: API pointer
    ctx->log->info("[AutoAttack] Combat enabled - Merlin taking control");
    return 0;
}

// FalconPM atcommand function (not rAthena ACMD_FUNC)
static int atcommand_autoattack(map_session_data* sd, const char* command, const char* message) {
    if (!sd) return -1;

    bool enable = false;

    if (!message || !*message) {
        extern void fpm_send_message(map_session_data* sd, const char* msg);
        fpm_send_message(sd, "Usage: @aa <on|off>");
        return -1;
    }

    if (strcmp(message, "on") == 0) {
        enable = true;
    } else if (strcmp(message, "off") == 0) {
        enable = false;
    } else {
        extern void fpm_send_message(map_session_data* sd, const char* msg);
        fpm_send_message(sd, "Usage: @aa <on|off>");
        return -1;
    }

    if (enable) {
        // Set FalconPM globals that Merlin expects
        extern int g_autoattack_account_id;
        extern struct GatMap* g_autoattack_map;
        // extern struct GatMap* gat_load(const char* filename); // OLD: raw global call (removed)

        g_autoattack_account_id = fpm_get_account_id(sd);

        // Load current map's GAT data for Merlin
        char gat_filename[256];
        int map_id = fpm_get_sd_m(sd);
        const char* map_name = fpm_get_map_name(map_id);
        snprintf(gat_filename, sizeof(gat_filename), "db/gat/%s.gat", map_name);

        // g_autoattack_map = gat_load(gat_filename); // OLD line (caused undefined symbol)
        g_autoattack_map = ctx->peregrine->load_gat(gat_filename); // NEW: use Peregrine API

        if (!g_autoattack_map) {
            extern void fpm_send_message(map_session_data* sd, const char* msg);
            fpm_send_message(sd, "AutoAttack: Failed to load map data");
            g_autoattack_account_id = -1;
            return -1;
        }

        // Activate Merlin roaming state
        // extern void mln_api_set_state(int state); mln_api_set_state(1); // OLD: global, unresolved
        ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_ROAMING); // NEW: API pointer

        extern void fpm_send_message(map_session_data* sd, const char* msg);
        fpm_send_message(sd, "AutoAttack: Enabled - Merlin taking control");

    } else {
        // Disable FalconPM system
        extern int g_autoattack_account_id;
        extern struct GatMap* g_autoattack_map;
        // extern void gat_free(struct GatMap* g); // OLD: raw global call (removed)
        // extern void mln_api_set_state(int state); // OLD: global, unresolved

        // mln_api_set_state(0); // OLD
        ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_IDLE); // NEW: API pointer

        if (g_autoattack_map) {
            // gat_free(g_autoattack_map); // OLD line (caused undefined symbol)
            ctx->peregrine->free_gat(g_autoattack_map); // NEW: use Peregrine API
            g_autoattack_map = nullptr;
        }
        g_autoattack_account_id = -1;

        extern void fpm_send_message(map_session_data* sd, const char* msg);
        fpm_send_message(sd, "AutoAttack: Disabled");
    }

    return 0;
}

extern bool fpm_atcommand_register(const char* name, AtCmdFunc fn);

extern "C" {
bool plugin_init(const PluginContext* u) {
    (void)u;
    ctx = falconpm_get_context();

        // Register @aa via FalconPM command system
        ctx->atcommand->add("aa", cmd_aa);

        // NOTE: Removed fpm_atcommand_register duplicate registration
        printf("[AutoAttack] Plugin loaded - @aa command available\n");


    printf("[AutoAttack] Plugin loaded - @aa command available\n");
    return true;
}

void plugin_final() {
    if (g_autoattack_map) {
        ctx->peregrine->free_gat(g_autoattack_map);
        g_autoattack_map = nullptr;
    }
}

PluginDescriptor PLUGIN = {"autoattack", "0.4", nullptr, plugin_init, plugin_final};
}
