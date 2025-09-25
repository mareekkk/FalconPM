#include "lnr_state.h"
#include "lnr_api.h"
#include "../../infra/plugin_api.h"

// Forward declarations from bootstrap
extern "C" {
    bool fpm_skill_is_available(struct map_session_data* sd, uint16_t skill_id);
    void fpm_unit_skilluse_nodamage(struct map_session_data* sd, struct block_list* target,
                                    uint16_t skill_id, uint16_t skill_lv);
    int  fpm_get_account_id(struct map_session_data* sd);
}

// Accessor provided by lnr_api.cpp
extern "C" const PluginContext* lnr_get_context(void);

// Local state
static LannerState g_lanner_state = LNR_STATE_IDLE;
static struct map_session_data* g_active_player = nullptr;

// [PATCH] global enable flag
static bool g_lanner_enabled = false;

// [PATCH] Local mirror of rAthena sc_type values (from status.hpp)
#define FPM_SC_BLESSING      33
#define FPM_SC_INCREASEAGI   34
#define FPM_SC_KYRIE        57
#define FPM_SC_IMPOSITIO    59
#define FPM_SC_GLORIA       60
#define FPM_SC_SUFFRAGIUM   61
#define FPM_SC_MAGNIFICAT   62

// BuffEntry with sc_type
struct BuffEntry {
    uint16_t skill_id;
    uint16_t skill_lv;
    int sc_type;
};

// Buff list (skill_id, skill_lv, sc_type)
static BuffEntry g_buff_list[] = {
    { 34, 10, FPM_SC_BLESSING },      // AL_BLESSING – STR/DEX/INT up
    { 29, 10, FPM_SC_INCREASEAGI },   // AL_INCAGI – AGI & movespeed up
    { 75, 10, FPM_SC_KYRIE },         // PR_KYRIE – Shield against hits
    { 66, 5,  FPM_SC_IMPOSITIO },     // PR_IMPOSITIO – ATK boost
    { 67, 5,  FPM_SC_GLORIA },        // PR_GLORIA – LUK +30
    { 69, 5,  FPM_SC_MAGNIFICAT },    // PR_MAGNIFICAT – SP regen double
    { 68, 3,  FPM_SC_SUFFRAGIUM },    // PR_SUFFRAGIUM – Next cast time reduced
};
static size_t g_current_buff_index = 0;

// Buff delay tracking
static uint64_t next_buff_time = 0;
static const uint64_t buff_cast_delay = 1200; // 1.2s

// Throttled tick logging
static uint64_t last_tick_log = 0;
static const uint64_t tick_log_delay = 5000; // 5s

// Helper: check if all buffs are active
static bool all_buffs_active(struct map_session_data* sd, const PluginContext* ctx) {
    for (size_t i = 0; i < sizeof(g_buff_list) / sizeof(g_buff_list[0]); ++i) {
        if (!ctx->status->has_status(sd, g_buff_list[i].sc_type))
            return false;
    }
    return true;
}

// --- API Implementation ---

extern "C" void lanner_tick(void) {
    const PluginContext* ctx = lnr_get_context();
    if (!ctx || !ctx->log) return;

    // [PATCH] Early exit if disabled or no player
    if (!g_lanner_enabled || !g_active_player) return;

    uint64_t now = ctx->timer->gettick();

    // Throttle logs
    if (now - last_tick_log > tick_log_delay) {
        ctx->log->info("[Lanner] Tick - State=%d", (int)g_lanner_state);
        last_tick_log = now;
    }

    switch (g_lanner_state) {
    case LNR_STATE_IDLE: {
        if (!all_buffs_active(g_active_player, ctx)) {
            BuffEntry& buff = g_buff_list[g_current_buff_index];

            // [PATCH] Only try to cast if delay window has passed
            if (now >= next_buff_time) {
                if (!ctx->status->has_status(g_active_player, buff.sc_type)) {
                    if (fpm_skill_is_available(g_active_player, buff.skill_id)) {
                        struct block_list* target = (struct block_list*)g_active_player;
                        fpm_unit_skilluse_nodamage(g_active_player, target,
                                                   buff.skill_id, buff.skill_lv);

                        ctx->log->info("[Lanner] Buff cast initiated: skill=%d", buff.skill_id);
                        g_lanner_state = LNR_STATE_BUFFING;

                        // [PATCH] set delay window
                        next_buff_time = now + buff_cast_delay;
                    } else {
                        ctx->log->info("[Lanner] Buff skill=%d not available, skipping", buff.skill_id);
                        // still advance index if unavailable
                        g_current_buff_index = (g_current_buff_index + 1) %
                                               (sizeof(g_buff_list) / sizeof(g_buff_list[0]));
                    }
                } else {
                    // Buff already active, move on
                    g_current_buff_index = (g_current_buff_index + 1) %
                                           (sizeof(g_buff_list) / sizeof(g_buff_list[0]));
                }
            }
        }
        break;
    }

    case LNR_STATE_BUFFING: {
        // [PATCH] Stay here until delay expires, then advance index
        if (now >= next_buff_time) {
            g_current_buff_index = (g_current_buff_index + 1) %
                                   (sizeof(g_buff_list) / sizeof(g_buff_list[0]));
            g_lanner_state = LNR_STATE_IDLE;
            ctx->log->info("[Lanner] Buff delay elapsed - returning to IDLE");
        }
        break;
    }

    case LNR_STATE_CASTING: {
        ctx->log->info("[Lanner] CASTING state - returning to IDLE");
        g_lanner_state = LNR_STATE_IDLE;
        break;
    }

    default:
        ctx->log->error("[Lanner] Unknown state=%d", (int)g_lanner_state);
        g_lanner_state = LNR_STATE_IDLE;
        break;
    }
}

extern "C" bool lanner_is_active(void) {
    return (g_active_player != nullptr && g_lanner_enabled &&
            g_lanner_state != LNR_STATE_IDLE);
}

extern "C" void lanner_start(struct map_session_data* sd) {
    const PluginContext* ctx = lnr_get_context();

    g_active_player = sd;
    g_lanner_state = LNR_STATE_IDLE;
    g_current_buff_index = 0;
    next_buff_time = 0;
    g_lanner_enabled = true; // enable

    if (ctx && ctx->log) {
        ctx->log->info("[Lanner] Started for player account_id=%d",
                       sd ? fpm_get_account_id(sd) : -1);
    }
}

extern "C" void lanner_stop(void) {
    const PluginContext* ctx = lnr_get_context();

    g_active_player = nullptr;
    g_lanner_state = LNR_STATE_IDLE;
    g_current_buff_index = 0;
    next_buff_time = 0;
    g_lanner_enabled = false; // disable

    if (ctx && ctx->log) {
        ctx->log->info("[Lanner] Stopped");
    }
}

// New helpers for Merlin / orchestration

extern "C" bool lanner_buffs_ready(struct map_session_data* sd) {
    const PluginContext* ctx = lnr_get_context();
    if (!ctx || !sd) return false;
    return all_buffs_active(sd, ctx);
}

extern "C" void lanner_request_buffs(struct map_session_data* sd) {
    g_active_player = sd;
    g_lanner_state = LNR_STATE_IDLE;
    g_lanner_enabled = true;
}
