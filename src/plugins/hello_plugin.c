#include "plugin_api.h"
#include <stdio.h>

static PluginAPI* g_api = NULL;

void plugin_init(PluginAPI* api) {
    g_api = api;

    if (g_api->log_info) {
        g_api->log_info("[HelloPlugin] Initialized!\n");
    } else {
        printf("[HelloPlugin] HelloPlugin initialized (no log_info available)\n");
    }
}