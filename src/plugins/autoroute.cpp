// src/plugins/autoroute.cpp

#include "map/pc.hpp"
#include "map/map.hpp"
#include "map/clif.hpp"
#include "common/showmsg.hpp"
#include "plugin_api.h"    // FalconPM API

#define VAR_AR_ENABLED     "#ar_enabled"
#define VAR_WANDER_ENABLED "#wander_enabled"

// @ar <x> <y>
static int atcommand_ar(int fd, struct map_session_data *sd, const char *command, const char *message) {
    int x, y;
    if (sscanf(message, "%d %d", &x, &y) != 2) {
        g_plugin_api->send_message(fd, "Usage: @ar <x> <y>");
        return -1;
    }

    g_plugin_api->accountvar_set(sd, VAR_AR_ENABLED, 1);
    ShowInfo("[autoroute] Target set (%d,%d)\n", x, y);
    return 0;
}

// @wander on|off
static int atcommand_wander(int fd, struct map_session_data* sd, const char* command, const char* message) {
    if (strcmp(message, "on") == 0) {
        g_plugin_api->accountvar_set(sd, VAR_WANDER_ENABLED, 1);
        g_plugin_api->send_message(fd, "[autoroute] Wander mode enabled.");
    } else if (strcmp(message, "off") == 0) {
        g_plugin_api->accountvar_set(sd, VAR_WANDER_ENABLED, 0);
        g_plugin_api->send_message(fd, "[autoroute] Wander mode disabled.");
    } else {
        g_plugin_api->send_message(fd, "Usage: @wander on|off");
    }
    return 0;
}

static void autoroute_tick(struct map_session_data* sd) {
    int wander = g_plugin_api->accountvar_get(sd, VAR_WANDER_ENABLED);
    if (wander) {
        g_plugin_api->log_info("[autoroute] Wander tick for %s\n", sd->status.name);
    }
}

// plugin_init
extern "C" void plugin_init(void) {
    ShowInfo("[autoroute] plugin_init\n");
    g_plugin_api->register_atcommand("ar", atcommand_ar);
    g_plugin_api->register_atcommand("wander", atcommand_wander);
    falconpm_register("autoroute", autoroute_tick);
}
