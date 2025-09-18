/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: loader.cpp
 *  Description: FalconPM dynamic plugin loader — loads .so plugins into rAthena at runtime
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

#include "plugin_api.hpp"
#include <dlfcn.h>
#include <filesystem>
#include <string>
#include <iostream>

// Global API table
PluginAPI api;
PluginAPI* g_plugin_api = &api;

using plugin_init_func = void(*)(PluginAPI*);

// Stub implementations (replace with real rAthena hooks later)
static void log_info_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

static unsigned int gettick_impl() {
    return 12345; // stub, later map to rAthena's gettick()
}

static void add_timer_impl(unsigned int tick, void(*cb)(void*), void* data) {
    printf("[FalconPM] Timer scheduled at %u\n", tick);
    // later map to rAthena timer system
}

// Plugin loader
void falconpm_load_plugins() {
    // Fill API table
    api.log_info  = log_info_impl;
    api.gettick   = gettick_impl;
    api.add_timer = add_timer_impl;

    std::string path = "plugins"; // plugin folder
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
                std::cout << "[FalconPM] Loaded plugin: " << entry.path().filename() << "\n";
            }
        }
    }
}
