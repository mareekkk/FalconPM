/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: loader.cpp
 *  Description: FalconPM dynamic plugin loader — loads .so plugins at runtime
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  Licensed under GNU General Public License v3 or later.
 *  See <https://www.gnu.org/licenses/>.
 */

#include "plugin_api.hpp"
#include <dlfcn.h>
#include <filesystem>
#include <string>
#include <iostream>

// Global API table
PluginAPI api;
PluginAPI* g_plugin_api = &api;

// Plugin entrypoint type
using plugin_init_func = void(*)(PluginAPI*);

// --- Stub implementations (replace with real rAthena hooks later) ---
static void log_info_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static uint32_t gettick_impl() {
    // TODO: replace with rAthena's gettick()
    return 12345;
}

static void add_timer_impl(uint32_t tick, void(*cb)(void*), void* data) {
    // TODO: replace with rAthena's timer system
    printf("[FalconPM] Timer scheduled at %u\n", tick);
    if (cb) {
        cb(data); // fire immediately for demo
    }
}

// --- Loader implementation ---
void falconpm_load_plugins() {
    // Fill API table
    api.log_info  = log_info_impl;
    api.gettick   = gettick_impl;
    api.add_timer = add_timer_impl;

    std::string path = "plugins"; // runtime plugin folder
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() == ".so") {
            void* handle = dlopen(entry.path().c_str(), RTLD_NOW);
            if (!handle) {
                std::cerr << "[FalconPM] Failed to load: " << entry.path() << "\n";
                continue;
            }
            auto init = (plugin_init_func)dlsym(handle, "plugin_init");
            if (init) {
                init(&api);
                std::cout << "[FalconPM] Loaded plugin: "
                          << entry.path().filename().string() << "\n";
            }
        }
    }
}
