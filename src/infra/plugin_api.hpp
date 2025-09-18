#pragma once
#include <cstdarg>
#include <cstdio>

// FalconPM API version
#define FALCONPM_API_VERSION 1

// Forward declarations (you can expand later)
struct Player;

// Function pointer table
struct PluginAPI {
    void (*log_info)(const char* fmt, ...);
    unsigned int (*gettick)();
    void (*add_timer)(unsigned int tick, void(*cb)(void*), void* data);
};

// Global API pointer accessible to plugins
extern PluginAPI* g_plugin_api;
