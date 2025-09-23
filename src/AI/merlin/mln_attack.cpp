#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstdint>

extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

// Bootstrap helper
extern "C" {
    bool fpm_is_mob_alive(struct block_list* mob);
    int  fpm_get_bl_id(struct block_list* bl);
}

// State
static bool     attack_active   = false;
static void*    current_target  = nullptr;
static int      attack_counter  = 0;
static uint64_t last_attack_time = 0;

extern const PluginContext* falconpm_get_context(void);

bool mln_attack_start(void* mob) {
    if (!mob) {
        printf("[Debug] mln_attack_start: null mob pointer\n");
        return false;
    }

    // If mob already dead, refuse start
    if (!fpm_is_mob_alive(reinterpret_cast<block_list*>(mob))) {
        printf("[Debug] mln_attack_start: mob %d not alive, cannot start\n", fpm_get_bl_id((block_list*)mob));
        return false;
    }

    current_target   = mob;
    attack_active    = true;
    attack_counter++;
    last_attack_time = 0;

    printf("[Merlin] Attack sequence #%d initiated on mob %d\n", attack_counter, fpm_get_bl_id((block_list*)mob));
    return true;
}

bool mln_attack_in_progress(void) {
    if (!attack_active || !current_target) return false;

    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return false;

    map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return false;

    block_list* mob = reinterpret_cast<block_list*>(current_target);

    // Stop if mob is dead now
    if (!fpm_is_mob_alive(mob)) {
        printf("[Debug] mln_attack_in_progress: mob %d is dead\n", fpm_get_bl_id(mob));
        attack_active   = false;
        current_target  = nullptr;
        return false;
    }

    // Issue an attack every 500ms
    const uint64_t now = ctx->timer->gettick();
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, mob);
        last_attack_time = now;

        if (result == 0) {
            printf("[Merlin] Attack executed on mob %d\n", fpm_get_bl_id(mob));
        } else {
            printf("[Debug] unit_attack returned %d on mob %d\n", result, fpm_get_bl_id(mob));
        }
    }

    return true;
}

bool mln_attack_done(void) {
    if (!attack_active) return true;

    block_list* mob = reinterpret_cast<block_list*>(current_target);
    if (!mob) {
        printf("[Debug] mln_attack_done: mob pointer null\n");
        attack_active = false;
        return true;
    }

    if (!fpm_is_mob_alive(mob)) {
        printf("[Debug] mln_attack_done: mob %d confirmed dead\n", fpm_get_bl_id(mob));
        attack_active  = false;
        current_target = nullptr;
        return true;
    }
    return false;
}
