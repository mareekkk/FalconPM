#pragma once
#include <cstdint>
#include "../common/timer.hpp" 

void falconpm_bootstrap(void);
void falconpm_shutdown(void);

extern "C" {
    int fpm_add_timer(uint64_t tick, TimerFunc func, int id, intptr_t data);
    uint64_t fpm_gettick(void);
}

// Attack state API
extern "C" {
    bool fpm_is_basicattack_active(int account_id);
    void fpm_set_basicattack_active(int account_id, bool active);
    void fpm_clear_basicattack_state(int account_id);
}
