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

// Forward declarations from rAthena
struct map_session_data;
struct block_list;
struct walkpath_data;
struct PStepList;
struct GatMap;

// -----------------------------
// Common header for all API tables
// -----------------------------
typedef struct {
    size_t size;          // sizeof(struct)
    FpmApiVersion ver;    // ABI version
} FpmTableHeader;

// ----------------------------------------------------
// Player movement API
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    int (*pc_walktoxy)(struct map_session_data* sd, short x, short y, int type);
    int (*unit_walktoxy)(struct block_list* bl, short x, short y, unsigned char flag);
} PlayerMovementAPI;

// ----------------------------------------------------
// Pathfinding
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    int (*path_search)(struct walkpath_data *wpd, int m,
                       int x0, int y0, int x1, int y1, int flag);
} PathAPI;

typedef struct {
    FpmTableHeader _;
    struct GatMap* (*load_gat)(const char* filename);
    void           (*free_gat)(struct GatMap* g);
    bool           (*is_walkable)(const struct GatMap* g, int x, int y);
    bool           (*astar)(const struct GatMap* g, int sx, int sy, int tx, int ty, struct PStepList* out);
    void           (*free_steps)(struct PStepList* l);
} SmartAPI;


// ----------------------------------------------------
// Direction API (dx/dy arrays for walkpath)
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    const int16_t* dx;  // pointer to dirx[DIR_MAX]
    const int16_t* dy;  // pointer to diry[DIR_MAX]
} DirectionAPI;

// ----------------------------------------------------
// Logging
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    void (*info)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
} LogAPI;

// ----------------------------------------------------
// Player
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    struct map_session_data* (*map_id2sd)(int aid);
    void (*send_message)(int fd, const char* msg);
} PlayerAPI;

// ----------------------------------------------------
// Unit
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    struct block_list* (*get_target)(void* u);
    int  (*get_id)(struct block_list* bl);
    int  (*get_type)(struct block_list* bl);
} UnitAPI;

// ----------------------------------------------------
// Random
// ----------------------------------------------------
typedef struct {
    FpmTableHeader _;
    int32_t (*rnd)(void);
} RandomAPI;

// ----------------------------------------------------
// Atcommand
// ----------------------------------------------------
typedef int (*AtCmdFunc)(struct map_session_data* sd, const char* command, const char* message);

typedef struct {
    FpmTableHeader _;
    bool (*add)(const char* name, AtCmdFunc func);
    bool (*remove)(const char* name);
} AtcommandAPI;

// ----------------------------------------------------
// Timer
// ----------------------------------------------------
typedef int (*FpmTimerFunc)(int tid, uint64_t tick, int id, intptr_t data);

typedef struct {
    FpmTableHeader _;
    int (*add_timer)(uint64_t tick, FpmTimerFunc func, int id, intptr_t data);
    uint64_t (*gettick)(void);
} TimerAPI;

// -----------------------------
// PluginContext
// -----------------------------
typedef struct {
    FpmApiVersion api;
    LogAPI*            log;
    UnitAPI*           unit;
    PlayerAPI*         player;
    RandomAPI*         rnd;
    AtcommandAPI*      atcommand;
    PlayerMovementAPI* movement;
    TimerAPI*          timer;
    PathAPI*           path;
    DirectionAPI*      dir;
    SmartAPI*          smart;
} PluginContext;

// -----------------------------
// PluginDescriptor
// -----------------------------
typedef struct {
    const char* name;
    const char* version;
    const int* (*required_modules)(size_t* count);
    bool (*init)(const PluginContext* ctx);
    void (*shutdown)(void);
} PluginDescriptor;

#ifdef __cplusplus
}
#endif

#endif // FALCONPM_PLUGIN_API_H
