// plugin_api.h
// FalconPM Plugin API (pure C ABI)

#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

// Versioning: lets plugins check compatibility
#define FALCONPM_API_VERSION 1

// Forward declaration
typedef struct PluginAPI PluginAPI;

// Function table definition
struct PluginAPI {
    void (*log_info)(const char* fmt, ...);
    void (*log_error)(const char* fmt, ...);
    uint32_t (*gettick)(void);
    void (*add_timer)(uint32_t tick, void (*cb)(void*), void* user);
};

// Global pointer (set by loader)
extern PluginAPI* g_plugin_api;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
