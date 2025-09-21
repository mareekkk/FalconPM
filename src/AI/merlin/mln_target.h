#pragma once
#include "../../infra/plugin_api.h"

typedef struct {
    bool valid;
    int mob_id;
    int x;
    int y;
} MobTarget;

MobTarget mln_target_find(struct map_session_data* sd, int hunt_item_id);
