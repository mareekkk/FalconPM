#include "../../../src/infra/plugin_api.hpp"

void my_timer_callback(void* data) {
    g_plugin_api->log_info("[HelloPlugin] Timer fired!\n");
}

extern "C" void plugin_init(PluginAPI* api) {
    g_plugin_api = api;
    g_plugin_api->log_info("[HelloPlugin] Initialized!\n");

    unsigned int now = g_plugin_api->gettick();
    g_plugin_api->add_timer(now + 500, my_timer_callback, NULL);
}
