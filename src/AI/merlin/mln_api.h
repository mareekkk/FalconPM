#pragma once
#include "../../infra/plugin_api.h"
#include "mln_target.h"
#include "mln_attack.h"

// AI API for Merlin
typedef struct {
    FpmTableHeader _;
    bool (*tick)(struct map_session_data* sd);
    void (*set_hunt_item)(int item_id);
    void (*clear_plan)(void);
} MerlinAPI;

// Global instance
extern MerlinAPI merlin_api;
