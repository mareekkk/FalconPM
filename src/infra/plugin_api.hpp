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


#pragma once
#include <cstdarg>
#include <cstdio>

// FalconPM API version
#define FALCONPM_API_VERSION 1

// Forward declarations (expand later as needed)
struct Player;

// Plugin API function table
struct PluginAPI {
    void (*log_info)(const char* fmt, ...);
    unsigned int (*gettick)();
    void (*add_timer)(unsigned int tick, void(*cb)(void*), void* data);
};

// Global API pointer accessible by plugins
extern PluginAPI* g_plugin_api;
