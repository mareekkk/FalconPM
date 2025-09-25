// src/AI/hunter/hunter.cpp
#include "hnt_state.h"
#include <iostream>
#include "../core/fpm_tick.h"

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_HUNTER  "\033[36m"  // Cyan

// Current Hunter state
static HunterState hunter_state = HunterState::IDLE;

// Tick timestamps (use FalconPM helper, no uint64_t)
static unsigned int last_tick = 0;
static unsigned int cooldown_ms = 500; // default cooldown

// Called by global AI runner every tick
void hunter_tick() {
    unsigned int now = fpm_gettick();

    switch (hunter_state) {
        case HunterState::IDLE:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=IDLE" << std::endl;
            hunter_state = HunterState::QUEUED;
            break;

        case HunterState::QUEUED:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=QUEUED" << std::endl;
            hunter_state = HunterState::RUNNING;
            break;

        case HunterState::RUNNING:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=RUNNING" << std::endl;
            last_tick = now;
            hunter_state = HunterState::COOLDOWN;
            break;

        case HunterState::COOLDOWN:
            if (now - last_tick >= cooldown_ms) {
                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " Cooldown finished" << std::endl;
                hunter_state = HunterState::IDLE;
            } else {
                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " State=COOLDOWN (waiting)" << std::endl;
            }
            break;
    }
}
