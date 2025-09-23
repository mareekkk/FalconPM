// /mnt/data/mln_attack.cpp
// Full replacement - Merlin attack module (hardening + safe pointer checks)

#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstdint>

// Reference core globals
extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

// External functions / bootstrap helpers we rely on (C linkage)
extern "C" {
    bool is_mob_engaged_by_other(int mob_id, int account_id);
    int  fpm_get_bl_id(block_list* bl);

    // From bootstrap: mob liveness & HP helpers
    bool fpm_is_mob_alive(block_list* mob);
    int  fpm_get_mob_hp_percent(block_list* mob);
}

// Attack state
static bool attack_active = false;
static void* current_target = nullptr;
static int attack_counter = 0;
static uint64_t last_attack_time = 0;
static uint64_t attack_start_time = 0;
static int attack_attempts = 0;

extern const PluginContext* falconpm_get_context(void);

bool mln_attack_start(void* mob) {
    if (!mob) return false;

    // Safety: verify pointer is for a live mob before binding
    if (!fpm_is_mob_alive((block_list*)mob)) {
        printf("[Merlin] Attack start aborted - target not alive\n");
        return false;
    }
    current_target = mob;
    attack_active = true;
    attack_counter++;
    attack_start_time = 0;
    attack_attempts = 0;
    last_attack_time = 0;

    printf("[Merlin] Attack sequence #%d initiated\n", attack_counter);
    return true;
}

bool mln_attack_in_progress(void) {
    if (!attack_active || !current_target) return false;

    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return false;

    struct map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return false;

    // Early pointer validation - do not touch the block_list if not alive
    if (!fpm_is_mob_alive((block_list*)current_target)) {
        printf("[Merlin] Target no longer alive - aborting attack\n");
        attack_active = false;
        current_target = nullptr;
        return false;
    }

    // Anti-KS: if someone else engaged, bail out
    int mob_id = fpm_get_bl_id((block_list*)current_target);
    if (is_mob_engaged_by_other(mob_id, g_autoattack_account_id)) {
        printf("[Merlin] Another player engaged our target (id=%d) - disengaging\n", mob_id);
        attack_active = false;
        current_target = nullptr;
        return false;
    }

    // Check HP percent if available
    int hp_pct = fpm_get_mob_hp_percent((block_list*)current_target);
    if (hp_pct <= 0) {
        printf("[Merlin] Target HP is 0%% (id=%d) - marking as dead\n", mob_id);
        attack_active = false;
        current_target = nullptr;
        return false;
    }

    uint64_t now = ctx->timer->gettick();
    if (attack_start_time == 0) {
        attack_start_time = now;
        attack_attempts = 0;
    }

    // Perform an attack action roughly every 500ms
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, (struct block_list*)current_target);
        last_attack_time = now;
        attack_attempts++;

        // Many unit_attack implementations return 0 for success; interpret
        // non-zero as a retry/failure indicator. We treat success by time/attempt heuristics.
        if (result != 0) {
            // If attack hasn't succeeded quickly, mark completed by heuristics
            if (now - attack_start_time > 1500 || attack_attempts >= 5) {
                printf("[Merlin] Attack considered complete by heuristics (id=%d)\n", mob_id);
                attack_active = false;
                current_target = nullptr;
                return false;
            }
        } else {
            // If unit_attack returned success (0), still allow retry until hp->0 detected
            // but log the event
            printf("[Merlin] Attack executed (attempt %d) on id=%d\n", attack_attempts, mob_id);
        }
    }

    // Throttled debug message
    static uint64_t last_debug_log = 0;
    if (now - last_debug_log > 2000) {
        printf("[Debug] Attack in progress - attempt %d on id=%d\n", attack_attempts, mob_id);
        last_debug_log = now;
    }

    // Timeout fallback to avoid infinite lockups
    if (now - attack_start_time > 10000) {
        printf("[Merlin] Attack timeout on id=%d - aborting\n", mob_id);
        attack_active = false;
        current_target = nullptr;
        return false;
    }

    return true;
}

bool mln_attack_done(void) {
    if (!attack_active) {
        attack_start_time = 0;
        attack_attempts = 0;
        return true;
    }
    return false;
}
