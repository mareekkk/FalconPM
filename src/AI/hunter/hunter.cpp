// src/AI/hunter/hunter.cpp
#include "hnt_state.h"
#include "hnt_task.h"
#include "hnt_api.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include "../core/fpm_tick.h"

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_HUNTER  "\033[36m"  // Cyan

// Current Hunter state
static HunterState hunter_state = HunterState::IDLE;

// Tick timestamps
static unsigned int last_tick = 0;
static unsigned int cooldown_ms = 500; // default cooldown

// Task queue
static std::vector<std::shared_ptr<HunterTask>> task_queue;

// Public API: enqueue a task
void hunter_enqueue(std::shared_ptr<HunterTask> task) {
    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
              << " Task queued: " << task->name() << std::endl;
    task->enqueue_tick = fpm_gettick();
    task_queue.push_back(task);

    if (hunter_state == HunterState::IDLE)
        hunter_state = HunterState::QUEUED;
}

// Called by global AI runner every tick
void hunter_tick() {
    unsigned int now = fpm_gettick();

    switch (hunter_state) {
        case HunterState::IDLE:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=IDLE" << std::endl;
            break;

        case HunterState::QUEUED:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=QUEUED" << std::endl;

            if (!task_queue.empty()) {
                // Pick highest priority
                auto it = std::max_element(task_queue.begin(), task_queue.end(),
                    [](auto &a, auto &b) { return a->priority < b->priority; });

                auto task = *it;
                task_queue.erase(it);

                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " Executing " << task->name() << std::endl;

                if (task->execute()) {
                    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                              << " Task completed: " << task->name() << std::endl;
                } else {
                    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                              << " Task failed: " << task->name() << std::endl;
                }

                last_tick = now;
                cooldown_ms = task->cooldown_ms;
                hunter_state = HunterState::COOLDOWN;
            } else {
                hunter_state = HunterState::IDLE;
            }
            break;

        case HunterState::RUNNING:
            // We skip this; tasks are executed immediately in QUEUED
            hunter_state = HunterState::IDLE;
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
__attribute__((constructor))
static void hunter_module_init() {
    hnt_api_init();
}
