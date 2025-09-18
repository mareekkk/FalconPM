/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: falconpm.c
 *  Description: Provides tick loop + registry for plugins + humanized helpers
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "map/pc.h"
#include "map/status.h"
#include "map/script.h"
#include "map/map.h"
#include <stdlib.h>
#include <time.h>

#define MAX_FALCONPM_PLUGINS 32
typedef void (*falconpm_callback)(struct map_session_data *sd);

static falconpm_callback falconpm_plugins[MAX_FALCONPM_PLUGINS];
static int falconpm_count = 0;

// ------------------------------------------------------------
// Humanization helpers
// ------------------------------------------------------------

// Adds a randomized delay (simulates reaction time, avoids instant actions)
static void falconpm_delay(int min_ms, int max_ms) {
    int ms = min_ms + rand() % (max_ms - min_ms + 1);
    timer->add(timer->gettick() + ms, NULL, 0, 0);
}

// Random chance to skip an action (simulates hesitation/idle behavior)
static bool falconpm_skip(int percent) {
    return (rand() % 100) < percent;
}

// Adds jitter around a base value (simulates inconsistent player thresholds)
static int falconpm_jitter(int base, int spread) {
    return base + (rand() % (2*spread + 1)) - spread;
}

// ------------------------------------------------------------
// Register plugin
// ------------------------------------------------------------
void falconpm_register(const char *name, falconpm_callback cb) {
    if (falconpm_count < MAX_FALCONPM_PLUGINS) {
        falconpm_plugins[falconpm_count++] = cb;
        ShowInfo("[FalconPM] Registered plugin: %s\n", name);
    }
}

// ------------------------------------------------------------
// Tick loop — executed every 500ms for each active player
// ------------------------------------------------------------
static TIMER_FUNC(falconpm_tick) {
    struct map_session_data *sd;
    int i;

    for (sd = map->first_session; sd; sd = sd->next) {
        if (!sd->fd || sd->state.autotrade) continue;
        if (pc_readaccountreg(sd, script->add_str("#falconpm_enabled")) <= 0) continue;

        // Call all registered plugin ticks
        for (i = 0; i < falconpm_count; i++)
            falconpm_plugins[i](sd);
    }
    return 0;
}

// ------------------------------------------------------------
// Init
// ------------------------------------------------------------
void falconpm_init(void) {
    srand((unsigned)time(NULL));
    timer->add_func_list(falconpm_tick, "falconpm_tick");
    timer->add_interval(timer->gettick() + 500, falconpm_tick, 0, 0, 500);
    ShowInfo("[FalconPM] Base initialized (tick=500ms)\n");
}

HPExport void plugin_init(void) {
    falconpm_init();
}
