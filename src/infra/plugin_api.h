// plugin_api.h
// FalconPM Plugin API (pure C ABI)

#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#include <stdint.h>   // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

#define FALCONPM_API_VERSION 1

typedef struct PluginAPI {
    void     (*log_info)(const char* fmt, ...);
    void     (*log_error)(const char* fmt, ...);
    uint32_t (*gettick)(void);
    int      (*add_timer)(uint32_t tick, void (*cb)(void*), void* data);
} PluginAPI;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
