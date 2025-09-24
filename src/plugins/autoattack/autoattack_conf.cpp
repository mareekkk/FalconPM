// src/plugins/autoattack/autoattack_conf.cpp
// FalconPM AutoAttack Config Plugin - @conf menu

#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include "../../AI/saker/skr_conf.h"
#include "../../AI/merlin/mln_api.h"
#include <cstdio>
#include <unordered_set>

// External declarations from bootstrap
extern "C" {
    int fpm_get_bl_id(struct block_list* bl);
    void fpm_send_message(struct map_session_data* sd, const char* msg);
}

// Helper functions from skr_conf.cpp
extern "C" {
    void fpm_conf_set_filter(const std::unordered_set<int>& ids);
    const std::unordered_set<int>& fpm_conf_get_filter();
}

extern int g_autoattack_account_id;

// --------------------------------------------------------------------
// Callback for config menu
// --------------------------------------------------------------------
static void aa_conf_callback(int account_id, int choice) {
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx) return;
    
    switch (choice) {
    case 1: { // Filter Monster
        map_session_data* sd = ctx->player->map_id2sd(account_id);
        if (!sd) return;

        block_list* mob = ctx->combat->get_nearest_mob(sd, 30);
        if (mob) {
            int mob_id = fpm_get_bl_id(mob);
            bool allowed = fpm_mob_allowed(mob_id);

            std::unordered_set<int> filter = fpm_conf_get_filter();
            if (allowed) {
                filter.erase(mob_id);
            } else {
                filter.insert(mob_id);
            }
            fpm_conf_set_filter(filter);

            ctx->clif->scriptmes(sd, "[Filter Monster]");
            ctx->clif->scriptmes(sd, allowed ? "Mob removed from filter" : "Mob added to filter");
            ctx->clif->scriptnext(sd);
        } else {
            ctx->clif->scriptmes(sd, "No mobs nearby to filter.");
            ctx->clif->scriptnext(sd);
        }
        break;
    }

    case 2: { // Show Monster Filtered
        const auto& filter = fpm_conf_get_filter();
        map_session_data* sd = ctx->player->map_id2sd(account_id);
        if (!sd) return;

        ctx->clif->scriptmes(sd, "=== Current Monster Filter ===");
        if (filter.empty()) {
            ctx->clif->scriptmes(sd, "(Empty = attack all mobs)");
        } else {
            for (int id : filter) {
                char buf[64];
                snprintf(buf, sizeof(buf), "- Mob ID %d", id);
                ctx->clif->scriptmes(sd, buf);
            }
        }
        ctx->clif->scriptnext(sd);
        break;
    }

    case 3: // Save
        fpm_conf_save(account_id);
        break;

    case 4: // Start
        fpm_conf_start(account_id);
        break;

    case 5: // Cancel
        printf("[AutoAttack] Config menu closed for account %d\n", account_id);
        break;

    default:
        printf("[AutoAttack] Invalid menu choice %d\n", choice);
        break;
    }
}

// --------------------------------------------------------------------
// @conf command handler
// --------------------------------------------------------------------
int cmd_conf(map_session_data* sd, const char* command, const char* message) {
    if (!sd) return -1;
    
    if (g_autoattack_account_id < 0) {
        fpm_send_message(sd, "[AutoAttack] No active account. Use @aa on first.");
        return 0;
    }

    const char* options[] = {
        "Filter Monster",
        "Show Monster Filtered",
        "Save",
        "Start",
        "Cancel"
    };

    auto ctx = falconpm_get_context();
    if (!ctx || !ctx->menu) return -1;
    
    ctx->menu->open_menu(
        g_autoattack_account_id,
        "=== AutoAttack Config ===",
        options,
        5,
        aa_conf_callback
    );
    
    return 0;
}

// --------------------------------------------------------------------
// Plugin init
// --------------------------------------------------------------------

extern "C" {
    bool autoattack_conf_init() {
        auto ctx = falconpm_get_context();
        if (!ctx || !ctx->atcommand) return false;

        // Register @conf command
        ctx->atcommand->add("conf", cmd_conf);

        return true;
    }
}