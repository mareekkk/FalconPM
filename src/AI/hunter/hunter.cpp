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

// ----------------------------------------------------
// DummyTask (log-only test task)
// ----------------------------------------------------
class DummyTask : public HunterTask {
public:
    DummyTask() : HunterTask(static_cast<HunterTaskType>(0), 1, 500) {}
    std::string name() const override { return "DummyTask"; }
    bool execute() override {
        std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                  << " Executing DummyTask" << std::endl;
        return true;
    }
};

// ----------------------------------------------------
// Hunter State Machine
// ----------------------------------------------------
static HunterState hunter_state = HunterState::IDLE;
static unsigned int last_tick = 0;
static unsigned int cooldown_ms = 500;

static std::vector<std::shared_ptr<HunterTask>> task_queue;
static std::shared_ptr<HunterTask> current_task = nullptr;

// Boot test flag
static bool boot_test_injected = false;

// Public API: enqueue a task
void hunter_enqueue(std::shared_ptr<HunterTask> task) {
    if (!task) return;
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

            // Inject dummy task on first tick only
            if (!boot_test_injected) {
                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " Test inject: queueing DummyTask" << std::endl;
                hunter_enqueue(std::make_shared<DummyTask>());
                boot_test_injected = true;
            }
            break;

        case HunterState::QUEUED:
            std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                      << " State=QUEUED (queued=" << task_queue.size() << ")"
                      << std::endl;

            if (!task_queue.empty()) {
                // Pick highest priority
                auto it = std::max_element(task_queue.begin(), task_queue.end(),
                    [](auto &a, auto &b) { return a->priority < b->priority; });

                current_task = *it;
                task_queue.erase(it);

                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " Popped: " << current_task->name()
                          << " (prio=" << current_task->priority
                          << ", cd=" << current_task->cooldown_ms << "ms)"
                          << std::endl;

                hunter_state = HunterState::RUNNING;
            } else {
                hunter_state = HunterState::IDLE;
            }
            break;

        case HunterState::RUNNING:
            if (current_task) {
                if (current_task->execute()) {
                    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                              << " Task completed: " << current_task->name()
                              << std::endl;
                } else {
                    std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                              << " Task failed: " << current_task->name()
                              << std::endl;
                }
                last_tick = now;
                cooldown_ms = current_task->cooldown_ms;
                current_task.reset();
                hunter_state = HunterState::COOLDOWN;
            } else {
                hunter_state = HunterState::IDLE;
            }
            break;

        case HunterState::COOLDOWN:
            if (now - last_tick >= cooldown_ms) {
                std::cout << COLOR_HUNTER << "[Hunter]" << COLOR_RESET
                          << " Cooldown finished → IDLE" << std::endl;
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
