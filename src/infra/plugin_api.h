#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------
// FalconPM ABI versioning
// ---------------------------------------------------------------------
#define FPM_API_VERSION 1   // bump this if the PluginAPI struct changes

struct PluginAPI {
    uint32_t abi_version;   // must equal FPM_API_VERSION

    // Logging
    void (*log_info)(const char* fmt, ...);
    void (*log_error)(const char* fmt, ...);

    // Timer
    uint32_t (*gettick)(void);
    int (*add_timer_interval)(uint32_t when,
                              int (*cb)(int, uint32_t, int, intptr_t),
                              int id, intptr_t data, uint32_t interval);

    // PlayerAPI
    struct map_session_data* (*map_id2sd)(int aid);
    int (*pc_readregistry)(struct map_session_data* sd, int id);
    void (*pc_setregistry)(struct map_session_data* sd, int id, int val);

    // ClifAPI
    void (*clif_displaymessage)(int fd, const char* msg);
};

// ---------------------------------------------------------------------
// Every plugin must export this
// ---------------------------------------------------------------------
struct Plugin {
    const char* name;   // e.g. "trade_echo"
    const char* author;
    const char* description;
    uint32_t required_api_version; // must equal FPM_API_VERSION

    int (*on_load)(struct PluginAPI* api);   // called when plugin loads
    void (*on_unload)(void);                 // called when plugin unloads
};

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
