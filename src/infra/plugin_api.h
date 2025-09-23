#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------
// Forward declarations
// -----------------------------
struct map_session_data;
struct block_list;
struct walkpath_data;
typedef struct PStepList PStepList;
struct GatMap;
struct PluginContext;

// -----------------------------
// Versioning
// -----------------------------
#define FPM_API_VERSION_MAJOR 1
#define FPM_API_VERSION_MINOR 0

typedef struct {
    uint16_t major;
    uint16_t minor;
} FpmApiVersion;

// -----------------------------
// Common header
// -----------------------------
typedef struct {
    size_t size;
    FpmApiVersion ver;
} FpmTableHeader;

// -----------------------------
// Peregrine API
// -----------------------------
typedef struct PeregrineAPI {
    FpmTableHeader _;
    struct GatMap* (*load_gat)(const char* filename);
    void           (*free_gat)(struct GatMap* g);
    bool           (*is_walkable)(const struct GatMap* g, int x, int y);
    bool           (*astar)(const struct GatMap* g, int sx, int sy, int tx, int ty, struct PStepList* out);
    void           (*free_steps)(struct PStepList* l);
    void           (*route_start)(const struct PluginContext* ctx,
                                  struct map_session_data* sd,
                                  PStepList* steps,
                                  struct GatMap* g);
    void           (*route_stop)(void);
    bool           (*route_active)(void);
    void           (*tick)(void);
} PeregrineAPI;

// -----------------------------
// Merlin API
// -----------------------------
typedef struct MerlinAPI {
    void (*tick)(void);
    void* (*target_find)(void);
    bool (*attack_start)(void* mob);
    bool (*attack_in_progress)(void);
    bool (*attack_done)(void);
} MerlinAPI;


// -----------------------------
// Logging API
// -----------------------------
typedef struct LogAPI {
    FpmTableHeader _;
    void (*info)(const char* fmt, ...);
    void (*error)(const char* fmt, ...);
} LogAPI;

// -----------------------------
// Unit API
// -----------------------------
typedef struct UnitAPI {
    FpmTableHeader _;
    struct block_list* (*get_target)(void* u);
    int  (*get_id)(struct block_list* bl);
    int  (*get_type)(struct block_list* bl);
} UnitAPI;

// -----------------------------
// Player API
// -----------------------------
typedef struct PlayerAPI {
    FpmTableHeader _;
    struct map_session_data* (*map_id2sd)(int aid);
    void (*send_message)(struct map_session_data* sd, const char* msg);
    int (*get_account_id)(struct map_session_data* sd);
} PlayerAPI;

// -----------------------------
// Random API
// -----------------------------
typedef struct RandomAPI {
    FpmTableHeader _;
    int32_t (*rnd)(void);
} RandomAPI;

// -----------------------------
// Atcommand API
// -----------------------------
typedef int (*AtCmdFunc)(struct map_session_data* sd, const char* command, const char* message);

typedef struct AtcommandAPI {
    FpmTableHeader _;
    bool (*add)(const char* name, AtCmdFunc func);
    bool (*remove)(const char* name);
} AtcommandAPI;

// -----------------------------
// Player Movement API
// -----------------------------
typedef struct PlayerMovementAPI {
    FpmTableHeader _;
    int (*pc_walktoxy)(struct map_session_data* sd, short x, short y, int type);
    int (*unit_walktoxy)(struct block_list* bl, short x, short y, unsigned char flag);
} PlayerMovementAPI;

// -----------------------------
// Timer API
// -----------------------------
typedef int (*FpmTimerFunc)(int tid, uint64_t tick, int id, intptr_t data);

typedef struct TimerAPI {
    FpmTableHeader _;
    int (*add_timer)(uint64_t tick, FpmTimerFunc func, int id, intptr_t data);
    uint64_t (*gettick)(void);
} TimerAPI;

// -----------------------------
// Path API
// -----------------------------
typedef struct PathAPI {
    FpmTableHeader _;
    int (*path_search)(struct walkpath_data *wpd, int m,
                       int x0, int y0, int x1, int y1, int flag);
} PathAPI;

// -----------------------------
// Direction API
// -----------------------------
typedef struct DirectionAPI {
    FpmTableHeader _;
    const int16_t* dx;
    const int16_t* dy;
} DirectionAPI;

// -----------------------------
// Combat API
// -----------------------------
typedef struct CombatAPI {
    FpmTableHeader _;
    struct block_list* (*get_nearest_mob)(struct map_session_data* sd, int range);
    int (*unit_attack)(struct map_session_data* sd, struct block_list* target);
} CombatAPI;

// -----------------------------
// PluginContext
// -----------------------------
typedef struct PluginContext {
    FpmApiVersion api;
    struct LogAPI*            log;
    struct UnitAPI*           unit;
    struct PlayerAPI*         player;
    struct RandomAPI*         rnd;
    struct AtcommandAPI*      atcommand;
    struct PlayerMovementAPI* movement;
    struct TimerAPI*          timer;
    struct PathAPI*           path;
    struct DirectionAPI*      dir;
    struct PeregrineAPI*      peregrine;
    struct CombatAPI*         combat;
    struct MerlinAPI*         merlin;
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