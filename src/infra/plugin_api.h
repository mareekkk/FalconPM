#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// Versioning
// -----------------------------
#define FPM_API_VERSION_MAJOR 1
#define FPM_API_VERSION_MINOR 0

typedef struct {
    uint16_t major;
    uint16_t minor;
} FpmApiVersion;

// Forward decls
struct map_session_data;
struct block_list;

// -----------------------------
// Common header
// -----------------------------
typedef struct {
    uint32_t size;        // sizeof(struct)
    FpmApiVersion ver;    // ABI version
} FpmTableHeader;

// -----------------------------
// Logging
// -----------------------------
typedef struct {
    FpmTableHeader _;
    void (*info)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
} LogAPI;

// -----------------------------
// Player
// -----------------------------
typedef struct {
    FpmTableHeader _;
    struct map_session_data* (*map_id2sd)(int aid);
    void (*send_message)(int fd, const char* msg);
} PlayerAPI;

// -----------------------------
// Unit
// -----------------------------
typedef struct {
    FpmTableHeader _;
    struct block_list* (*get_target)(void* u);
    int  (*get_id)(struct block_list* bl);
    int  (*get_type)(struct block_list* bl);
} UnitAPI;

// -----------------------------
// Random
// -----------------------------
typedef struct {
    FpmTableHeader _;
    int32_t (*rnd)(void);
} RandomAPI;

// -----------------------------
// Atcommand
// -----------------------------
typedef int (*AtCmdFunc)(struct map_session_data* sd,
                         const char* command,
                         const char* message);

typedef struct {
    FpmTableHeader _;
    void (*add)(const char* cmd, AtCmdFunc func);
    void (*remove)(const char* cmd);
} AtcommandAPI;

// -----------------------------
// Module IDs
// -----------------------------
typedef enum {
    FPM_MOD_LOG,
    FPM_MOD_PLAYER,
    FPM_MOD_UNIT,
    FPM_MOD_RANDOM,
    FPM_MOD_ATCOMMAND,
    FPM_MOD__COUNT
} FpmModuleId;

// -----------------------------
// PluginContext
// -----------------------------
typedef struct {
    FpmApiVersion api;
    LogAPI*       log;
    UnitAPI*      unit;
    PlayerAPI*    player;
    RandomAPI*    rnd;
    AtcommandAPI* atcommand;
} PluginContext;

// -----------------------------
// PluginDescriptor
// -----------------------------
typedef struct {
    const char* name;
    const char* version;
    const FpmModuleId* (*required_modules)(size_t* count);
    bool (*init)(const PluginContext* ctx);
    void (*shutdown)(void);
} PluginDescriptor;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
