/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: hello_plugin.cpp
 *  Description: FalconPM sample plugin — Hello World (demo timer + logging via API)
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  Licensed under GNU General Public License v3 or later.
 *  See <https://www.gnu.org/licenses/>.
 */

#include "plugin_api.hpp"

// Simple timer callback
void my_timer_callback(void* data) {
    g_plugin_api->log_info("[HelloPlugin] Timer fired!\n");
}

// Plugin entrypoint — called by loader.cpp
extern "C" void plugin_init(PluginAPI* api) {
    g_plugin_api = api;
    g_plugin_api->log_info("[HelloPlugin] Initialized!\n");

    uint32_t now = g_plugin_api->gettick();
    g_plugin_api->add_timer(now + 500, my_timer_callback, nullptr);
}
