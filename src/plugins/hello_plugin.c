// hello_plugin.c
// Minimal example plugin

#include "plugin_api.h"
#include <stdio.h>

static PluginAPI* g_api = NULL;

void plugin_init(PluginAPI* api) {
    g_api = api;
    g_api->log_info("[HelloPlugin] Initialized!\n");
}
