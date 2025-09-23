#include "mln_attack.h"
#include "../../infra/plugin_api.h"
#include "../../core/falconpm.hpp"
#include <cstdio>
#include <cstdint>
#include "../peregrine/pgn_api.h"

extern int g_autoattack_account_id;
extern GatMap* g_autoattack_map;

extern "C" {
    bool fpm_is_mob_alive(struct block_list* mob);
    int  fpm_get_bl_id(struct block_list* bl);
}

static bool     attack_active   = false;
static void*    current_target  = nullptr;
static int      attack_counter  = 0;
static uint64_t last_attack_time = 0;

// 🔹 new: track consecutive failures
static int      fail_count      = 0;

extern const PluginContext* falconpm_get_context(void);

bool mln_attack_start(void* mob) {
    if (!mob) {
        printf("[Debug] mln_attack_start: null mob pointer\n");
        return false;
    }
    if (!fpm_is_mob_alive(reinterpret_cast<block_list*>(mob))) {
        printf("[Debug] mln_attack_start: mob %d not alive, cannot start\n",
               fpm_get_bl_id((block_list*)mob));
        return false;
    }

    current_target   = mob;
    attack_active    = true;
    attack_counter++;
    last_attack_time = 0;
    fail_count       = 0;

    printf("[Merlin] Attack sequence #%d initiated on mob %d\n",
           attack_counter, fpm_get_bl_id((block_list*)mob));
    return true;
}

bool mln_attack_in_progress(void) {
    if (!attack_active || !current_target) return false;

    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || g_autoattack_account_id < 0) return false;

    map_session_data* sd = ctx->player->map_id2sd(g_autoattack_account_id);
    if (!sd) return false;

    block_list* mob = reinterpret_cast<block_list*>(current_target);

    if (!fpm_is_mob_alive(mob)) {
        printf("[Debug] mln_attack_in_progress: mob %d is dead\n", fpm_get_bl_id(mob));
        attack_active   = false;
        current_target  = nullptr;
        return false;
    }

    const uint64_t now = ctx->timer->gettick();
    if (now - last_attack_time >= 500) {
        int result = ctx->combat->unit_attack(sd, mob);
        last_attack_time = now;

        if (result == 0) {
            printf("[Merlin] Attack executed on mob %d\n", fpm_get_bl_id(mob));
            fail_count = 0;
        } else {
            printf("[Debug] unit_attack returned %d on mob %d (fail count=%d)\n",
                   result, fpm_get_bl_id(mob), fail_count + 1);
            fail_count++;

            // micro-reposition if repeated failures
            if (fail_count >= 3) {
                int mx = fpm_get_bl_x(mob);
                int my = fpm_get_bl_y(mob);
                int sx = fpm_get_sd_x(sd);
                int sy = fpm_get_sd_y(sd);

                PStepList steps{};
                if (ctx->peregrine->astar &&
                    ctx->peregrine->astar(g_autoattack_map, sx, sy, mx, my, &steps)) {
                    ctx->peregrine->route_start(ctx, sd, &steps, g_autoattack_map);
                    printf("[Debug] mln_attack_in_progress: repositioning to mob %d at (%d,%d)\n",
                           fpm_get_bl_id(mob), mx, my);
                } else {
                    printf("[Debug] mln_attack_in_progress: reposition failed, aborting target %d\n",
                           fpm_get_bl_id(mob));
                    attack_active   = false;
                    current_target  = nullptr;
                    return false;
                }
                if (ctx->peregrine->free_steps) ctx->peregrine->free_steps(&steps);

                fail_count = 0; // reset after micro-move
            }
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
