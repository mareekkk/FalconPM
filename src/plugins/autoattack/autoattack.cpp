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
        ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_IDLE);
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
    
    ctx->merlin->set_state((void*)(intptr_t)MLN_STATE_ROAMING);
    ctx->log->info("[AutoAttack] Combat enabled - Merlin taking control");
    return 0;
}

extern "C" {
bool plugin_init(const PluginContext* u) {
    (void)u;
    ctx = falconpm_get_context();
    ctx->atcommand->add("aa", cmd_aa);
    return true;
}

void plugin_final() {
    if (g_autoattack_map) {
        ctx->peregrine->free_gat(g_autoattack_map);
    }
}

PluginDescriptor PLUGIN = {"autoattack", "0.4", nullptr, plugin_init, plugin_final};
}