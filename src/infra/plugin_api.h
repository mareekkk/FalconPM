#ifndef FALCONPM_PLUGIN_API_H
#define FALCONPM_PLUGIN_API_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------
// ABI versioning
// ---------------------------------------------------------------------
#define FPM_API_VERSION_MAJOR 1
#define FPM_API_VERSION_MINOR 0

typedef struct {
    uint16_t major;
    uint16_t minor;
} FpmApiVersion;

// Forward declarations of rAthena structs
struct map_session_data;
struct block_list;

// ---------------------------------------------------------------------
// Common table header
// ---------------------------------------------------------------------
typedef struct {
    uint32_t size;        // sizeof(this struct)
    FpmApiVersion ver;    // version of this table
} FpmTableHeader;

// ---------------------------------------------------------------------
// Example APIs (extend as needed)
// ---------------------------------------------------------------------

// Logging API
typedef struct {
    FpmTableHeader _;
    void (*info)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
} LogAPI;

// Unit API
typedef struct {
    FpmTableHeader _;
    struct block_list* (*get_target)(void* unit);
    int  (*get_id)(struct block_list* bl);
    int  (*get_type)(struct block_list* bl);
} UnitAPI;

// Player API
typedef struct {
    FpmTableHeader _;
    struct map_session_data* (*map_id2sd)(int aid);
    void (*send_message)(int fd, const char* msg);
} PlayerAPI;

// Random API
typedef struct {
    FpmTableHeader _;
    int32_t (*rnd)(void); // [0, RAND_MAX]
} RandomAPI;

// ---------------------------------------------------------------------
// Plugin context: what a plugin sees
// ---------------------------------------------------------------------
typedef enum {
    FPM_MOD_LOG,
    FPM_MOD_UNIT,
    FPM_MOD_PLAYER,
    FPM_MOD_RANDOM,
    FPM_MOD__COUNT
} FpmModuleId;

typedef struct {
    FpmApiVersion api;

    LogAPI*       log;
    UnitAPI*      unit;
    PlayerAPI*    player;
    RandomAPI*    rnd;
} PluginContext;

// ---------------------------------------------------------------------
// Plugin descriptor (exported symbol)
// ---------------------------------------------------------------------
typedef struct {
    const char* name;
    const char* version;

    // Return list of required modules
    const FpmModuleId* (*required_modules)(size_t* count);

    // Called when plugin loads (only required modules are non-null)
    bool (*init)(const PluginContext* ctx);

    // Called when plugin unloads
    void (*shutdown)(void);
} PluginDescriptor;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
