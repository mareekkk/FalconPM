#include "lnr_state.h"
#include "../../infra/plugin_api.h"

// Forward declarations from bootstrap
extern "C" {
    bool fpm_skill_is_available(struct map_session_data* sd, uint16_t skill_id);
    void fpm_unit_skilluse_nodamage(struct map_session_data* sd, struct block_list* target, 
                                    uint16_t skill_id, uint16_t skill_lv);
}

extern const PluginContext* falconpm_get_context(void);

// Local state
static LannerState g_lanner_state = LNR_STATE_IDLE;
static struct map_session_data* g_active_player = nullptr;

// Buff configuration  
struct BuffEntry {
    uint16_t skill_id;
    uint16_t skill_lv;
};

static BuffEntry g_buff_list[] = {
    { 29, 10 },  // Increase Agility
    { 34, 10 },  // Blessing
};
static size_t g_current_buff_index = 0;

// --- API Implementation (matches existing LannerAPI in plugin_api.h) ---

extern "C" void lanner_tick(void) {
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || !ctx->log) return;

    // If no active player, can't do anything
    if (!g_active_player) return;

    ctx->log->info("[Lanner] Tick - State=%d", (int)g_lanner_state);

    switch (g_lanner_state) {
    case LNR_STATE_IDLE: {
        // Try to cast next buff in rotation
        BuffEntry& buff = g_buff_list[g_current_buff_index];
        
        ctx->log->info("[Lanner] Attempting buff skill=%d lv=%d", buff.skill_id, buff.skill_lv);
        
        // Check if skill is available
        if (fpm_skill_is_available(g_active_player, buff.skill_id)) {
            // Cast the buff on self
            struct block_list* target = (struct block_list*)g_active_player;
            fpm_unit_skilluse_nodamage(g_active_player, target, buff.skill_id, buff.skill_lv);
            
            ctx->log->info("[Lanner] Buff cast initiated: skill=%d", buff.skill_id);
            g_lanner_state = LNR_STATE_BUFFING;
            
            // Move to next buff for next cycle
            g_current_buff_index = (g_current_buff_index + 1) % (sizeof(g_buff_list) / sizeof(g_buff_list[0]));
        } else {
            ctx->log->info("[Lanner] Buff skill=%d not available, trying next", buff.skill_id);
            // Move to next buff and try again next tick
            g_current_buff_index = (g_current_buff_index + 1) % (sizeof(g_buff_list) / sizeof(g_buff_list[0]));
        }
        break;
    }
    
    case LNR_STATE_BUFFING: {
        ctx->log->info("[Lanner] Buff cycle completed - returning to IDLE");
        g_lanner_state = LNR_STATE_IDLE;
        break;
    }
    
    case LNR_STATE_CASTING: {
        // Future: handle offensive skill casting
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
    return (g_active_player != nullptr && g_lanner_state != LNR_STATE_IDLE);
}

extern "C" void lanner_start(struct map_session_data* sd) {
    const PluginContext* ctx = falconpm_get_context();
    
    g_active_player = sd;
    g_lanner_state = LNR_STATE_IDLE;
    g_current_buff_index = 0;
    
    if (ctx && ctx->log) {
        ctx->log->info("[Lanner] Started for player account_id=%d", 
                       sd ? fpm_get_account_id(sd) : -1);
    }
}

extern "C" void lanner_stop(void) {
    const PluginContext* ctx = falconpm_get_context();
    
    g_active_player = nullptr;
    g_lanner_state = LNR_STATE_IDLE;
    g_current_buff_index = 0;
    
    if (ctx && ctx->log) {
        ctx->log->info("[Lanner] Stopped");
    }
}