#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct map_session_data;

// Callback tick type
typedef void (*falconpm_callback)(struct map_session_data *sd);

// Register plugin tick with FalconPM
void falconpm_register(const char *name, falconpm_callback cb);

// Atcommand handler type
typedef int (*AtCmdHandler)(int fd,
                            struct map_session_data* sd,
                            const char* command,
                            const char* message);

typedef struct PluginAPI {
    // Logging
    void     (*log_info)(const char* fmt, ...);
    void     (*log_error)(const char* fmt, ...);

    // Timers
    uint32_t (*gettick)(void);
    int      (*add_timer_interval)(uint32_t when_ms,
                                   int (*cb)(int tid, uint32_t now, int id, intptr_t data),
                                   int id,
                                   intptr_t data,
                                   uint32_t interval_ms);

    // Map lookups
    struct map_session_data* (*map_id2sd)(int account_id);

    // Atcommand registration
    void     (*register_atcommand)(const char* name, AtCmdHandler handler);

    // Message output
    void     (*send_message)(int fd, const char* msg);

    // ✅ New: Account variable access (FalconPM shim)
    int      (*accountvar_get)(struct map_session_data* sd, const char* name);
    void     (*accountvar_set)(struct map_session_data* sd, const char* name, int val);
} PluginAPI;

// Exposed to plugins
extern PluginAPI* g_plugin_api;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
