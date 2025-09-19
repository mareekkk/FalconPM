// hello_plugin.c
// Minimal example plugin

#include "plugin_api.h"
#include <stdio.h>

// Required entrypoint
void plugin_init(PluginAPI* api) {
    g_plugin_api = api; // hook the global pointer
    g_plugin_api->log_info("[HelloPlugin] Initialized!\n");
}
