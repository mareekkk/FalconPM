/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: plugin_api.hpp
 *  Description: FalconPM Plugin API — defines the interface exposed to plugins
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  Licensed under GNU General Public License v3 or later.
 *  See <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdarg>
#include <cstdio>
#include <cstdint>

// Bump this when you introduce breaking changes
#define FALCONPM_API_VERSION 1

// Forward declarations for opaque handles you may expose later
struct Player;
struct Map;

// Function pointer table exposed to plugins.
// Keep this surface *small and stable*; expand carefully with versioning.
struct PluginAPI {
    // Logging
    void (*log_info)(const char* fmt, ...);
    // (Optional later)
    // void (*log_warn)(const char* fmt, ...);
    // void (*log_error)(const char* fmt, ...);

    // Time / timers
    uint32_t (*gettick)();  // server tick (ms)
    void (*add_timer)(uint32_t tick, void(*cb)(void*), void* user);

    // --- Future API candidates (commented until implemented) ---
    // Player*   (*get_player_by_id)(int id);
    // void      (*player_send_message)(Player* pl, const char* msg);
    // void      (*register_command)(const char* cmd, void(*handler)(Player*, const char* args));
    // int       (*sql_query)(const char* q, ...);
    // void      (*register_event_hook)(int hook_id, void(*cb)(void* ctx));
};

// Global API pointer available to plugins after plugin_init().
// Defined in the loader (compiled into map-server).
extern PluginAPI* g_plugin_api;
