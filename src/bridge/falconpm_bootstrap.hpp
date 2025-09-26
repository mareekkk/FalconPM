#pragma once
#include <cstdint>
#include "../common/timer.hpp" 

void falconpm_bootstrap(void);
void falconpm_shutdown(void);

extern "C" {
    // ----------------------------------------------------------------
    // Timer API
    // ----------------------------------------------------------------
    int fpm_add_timer(uint64_t tick, TimerFunc func, int id, intptr_t data);
    uint64_t fpm_gettick(void);

    // ----------------------------------------------------------------
    // Attack state API
    // ----------------------------------------------------------------
    bool fpm_is_basicattack_active(int account_id);
    void fpm_set_basicattack_active(int account_id, bool active);
    void fpm_clear_basicattack_state(int account_id);

    // ----------------------------------------------------------------
    // Lanner Skill API (new)
    // ----------------------------------------------------------------
    struct map_session_data;
    struct block_list;

    void   fpm_unit_skilluse_damage(struct map_session_data* sd, struct block_list* target, uint16_t skill_id, uint16_t skill_lv);
    void   fpm_unit_skilluse_nodamage(struct map_session_data* sd, struct block_list* target, uint16_t skill_id, uint16_t skill_lv);
    bool   fpm_skill_is_available(struct map_session_data* sd, uint16_t skill_id);
    int32_t fpm_skill_get_cooldown(uint16_t skill_id, uint16_t skill_lv);
}
