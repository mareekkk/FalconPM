// src/AI/hunter/hnt_api.cpp
#include "hnt_api.h"
#include "hunter.h"
#include "hnt_task.h"

// Global API instance
HunterAPI g_hunter_api;

void hnt_api_init() {
    g_hunter_api.tick = &hunter_tick;

    // Strongly typed: HunterTask*
    g_hunter_api.enqueue_task = [](HunterTask* task) {
        if (!task) return;
        hunter_enqueue(std::shared_ptr<HunterTask>(task));
    };
}

// Ensure API is initialized when this module is loaded
__attribute__((constructor))
static void hunter_module_init() {
    hnt_api_init();
}