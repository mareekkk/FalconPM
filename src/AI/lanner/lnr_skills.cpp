#include "../../infra/plugin_api.h"
#include "lnr_state.h"

// Direct bootstrap function calls (no need for BootstrapAPI structure)
extern "C" {
    bool fpm_skill_is_available(struct map_session_data* sd, uint16_t skill_id);
    void fpm_unit_skilluse_nodamage(struct map_session_data* sd, struct block_list* target, 
                                    uint16_t skill_id, uint16_t skill_lv);
    void fpm_unit_skilluse_damage(struct map_session_data* sd, struct block_list* target, 
                                  uint16_t skill_id, uint16_t skill_lv);
    int32_t fpm_skill_get_cooldown(uint16_t skill_id, uint16_t skill_lv);
    uint64_t fpm_gettick(void);
    int fpm_get_bl_id(struct block_list* bl);  // ADDED: needed for target->id access
}

// Accessor function to get context (defined in lnr_api.cpp)
extern "C" const PluginContext* lnr_get_context(void);

// Cast a support/buff skill on target (usually self for buffs)
bool lnr_cast_buff(int skill_id, int skill_lv, struct map_session_data* sd) {
    const PluginContext* ctx = lnr_get_context();
    
    if (!ctx || !sd) {
        if (ctx && ctx->log) {
            ctx->log->error("[Lanner] Missing context or player session");
        }
        return false;
    }

    // Check if skill is available (not on cooldown, player has SP, etc.)
    if (!fpm_skill_is_available(sd, (uint16_t)skill_id)) {
        if (ctx->log) {
            ctx->log->info("[Lanner] Skill %d unavailable (cooldown/SP/requirements)", skill_id);
        }
        return false;
    }

    // For self-buffs, target is the player
    struct block_list* target = (struct block_list*)sd;
    
    // Cast the support skill (non-damage)
    fpm_unit_skilluse_nodamage(sd, target, (uint16_t)skill_id, (uint16_t)skill_lv);
    
    if (ctx->log) {
        ctx->log->info("[Lanner] Cast support skill=%d lv=%d on self", skill_id, skill_lv);
    }
    return true;
}

// Cast an offensive skill on a target
bool lnr_cast_damage(int skill_id, int skill_lv, struct map_session_data* sd, struct block_list* target) {
    const PluginContext* ctx = lnr_get_context();
    
    if (!ctx || !sd || !target) {
        if (ctx && ctx->log) {
            ctx->log->error("[Lanner] Missing context, player, or target for damage skill");
        }
        return false;
    }

    if (!fpm_skill_is_available(sd, (uint16_t)skill_id)) {
        if (ctx->log) {
            ctx->log->info("[Lanner] Damage skill %d unavailable", skill_id);
        }
        return false;
    }

    // Cast offensive skill
    fpm_unit_skilluse_damage(sd, target, (uint16_t)skill_id, (uint16_t)skill_lv);
    
    if (ctx->log) {
        ctx->log->info("[Lanner] Cast damage skill=%d lv=%d on target=%d", 
                       skill_id, skill_lv, fpm_get_bl_id(target));
    }
    return true;
}

// Get skill cooldown from database
int32_t lnr_get_skill_cooldown(int skill_id, int skill_lv) {
    return fpm_skill_get_cooldown((uint16_t)skill_id, (uint16_t)skill_lv);
}