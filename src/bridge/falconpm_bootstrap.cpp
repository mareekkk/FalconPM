// falconpm_bootstrap.cpp
// Lives inside rAthena map-server. Exports unmangled C wrappers that plugins can call.
// Also loads FalconPM loader .so from rAthena/plugins.

#include "pc.hpp"             // declares pc_walktoxy
#include "unit.hpp"           // declares unit_walktoxy
#include "common/showmsg.hpp" // ShowError/ShowInfo
#include <dlfcn.h>
#include "path.hpp"   // rAthena pathfinding (struct walkpath_data, path_search)
#include <unordered_map>
#include "falconpm_bootstrap.hpp"

#include "pc.hpp"
#include "unit.hpp"
#include "map.hpp"
#include <climits> // for INT_MAX


extern "C" bool path_checkcell(int16 m, int16 x, int16 y, int32 flag);

extern "C" {

// Wrapper for player movement
int fpm_pc_walktoxy(map_session_data* sd, short x, short y, int type) {
    if (!sd) return -1;
    // map_session_data inherits from block_list, so safe cast
    return unit_walktoxy(static_cast<block_list*>(sd), x, y, (unsigned char)type);
}

// Wrapper for generic unit movement
int fpm_unit_walktoxy(block_list* bl, short x, short y, unsigned char flag) {
    return unit_walktoxy(bl, x, y, flag);
}

} // extern "C"

// ---- Optional: bootstrap FalconPM loader ----
static void* falconpm_handle = nullptr;

void falconpm_bootstrap(void) {
    falconpm_handle = dlopen("plugins/falconpm_loader.so", RTLD_NOW | RTLD_LOCAL);
    if (!falconpm_handle) {
        ShowError("FalconPM: failed to load plugins/falconpm_loader.so: %s\n", dlerror());
        return;
    }
    using init_t = int(*)(void);
    dlerror();
    init_t init = (init_t)dlsym(falconpm_handle, "falconpm_loader_init");
    if (!init) init = (init_t)dlsym(falconpm_handle, "plugin_init");
    if (const char* err = dlerror()) {
        ShowError("FalconPM: missing init symbol: %s\n", err);
        return;
    }
    if (!init()) {
        ShowError("FalconPM: loader init returned failure\n");
        return;
    }
    ShowInfo("FalconPM: loader initialized.\n");
}

void falconpm_shutdown(void) {
    if (falconpm_handle) {
        using fin_t = void(*)(void);
        if (auto fin = (fin_t)dlsym(falconpm_handle, "falconpm_loader_final")) fin();
        dlclose(falconpm_handle);
        falconpm_handle = nullptr;
        ShowInfo("FalconPM: loader shutdown.\n");
    }
}

// ----- Timer

extern "C" {

int fpm_add_timer(uint64_t tick, TimerFunc func, int id, intptr_t data) {
    return add_timer((t_tick)tick, func, id, data);
}

uint64_t fpm_gettick(void) {
    return gettick();
}

} // extern "C"

//  ----- Pathing

extern "C" {
int fpm_path_search(struct walkpath_data *wpd, int m,
                    int x0, int y0, int x1, int y1, int flag) {
    return path_search(wpd,
                       (int16)m, (int16)x0, (int16)y0,
                       (int16)x1, (int16)y1,
                       (int32)flag,
                       (cell_chk)0); // ✅ "0" usually means default walkable check
}
}

// Direction arrays from rAthena core
extern const int16 dirx[DIR_MAX];
extern const int16 diry[DIR_MAX];

extern "C" {
    const int16* fpm_get_dirx() { return dirx; }
    const int16* fpm_get_diry() { return diry; }
}

// ----------------------------------------------------
// Autoattack toggle state (per account)
// ----------------------------------------------------

static std::unordered_map<int,bool> basicattack_state;

extern "C" {
bool fpm_is_basicattack_active(int account_id) {
    auto it = basicattack_state.find(account_id);
    return (it != basicattack_state.end() && it->second);
}

void fpm_set_basicattack_active(int account_id, bool active) {
    basicattack_state[account_id] = active;
}

void fpm_clear_basicattack_state(int account_id) {
    basicattack_state.erase(account_id);
}
}

// ----------------------------------------------------
// Combat helpers
// ----------------------------------------------------
extern "C" {

// Static storage for nearest mob search
static block_list* fpm_best_mob = nullptr;
static int fpm_best_dist = INT_MAX;

// Callback used by map_foreachinrange
static int32 fpm_nearest_mob_cb(block_list* bl, va_list ap) {
    if (!bl || bl->type != BL_MOB)
        return 0;

    block_list* center = va_arg(ap, block_list*);
    if (!center) return 0;

    int dx = bl->x - center->x;
    int dy = bl->y - center->y;
    int d2 = dx*dx + dy*dy;

    if (d2 < fpm_best_dist) {
        fpm_best_mob  = bl;
        fpm_best_dist = d2;
    }
    return 0; // continue iteration
}

// Find nearest mob around a player
block_list* fpm_get_nearest_mob(map_session_data* sd, int range) {
    if (!sd) return nullptr;

    block_list* center = static_cast<block_list*>(sd); // map_session_data inherits block_list
    fpm_best_mob  = nullptr;
    fpm_best_dist = INT_MAX;

    // Call rAthena iterator with our callback
    map_foreachinrange(fpm_nearest_mob_cb, center, (int16)range, BL_MOB, center);

    return fpm_best_mob;
}

extern "C" int fpm_get_account_id(map_session_data* sd) {
    if (!sd) return -1;
    return sd->status.account_id;
}

extern "C" void fpm_send_message(map_session_data* sd, const char* msg) {
    if (!sd || !msg) return;
    clif_displaymessage(sd->fd, msg);
}

// Trigger attack (continuous auto-attack)
int fpm_unit_attack(map_session_data* sd, block_list* target) {
    if (!sd || !target) return -1;
    block_list* src = static_cast<block_list*>(sd);
    // unit_attack(block_list* src, int32 target_id, int32 continuous)
    return unit_attack(src, static_cast<int32>(target->id), /*continuous=*/1);
}

} // extern "C"

// ----------------------------------------------------
// Map helpers (exposed to FalconPM plugins)
// ----------------------------------------------------
extern "C" {

int fpm_get_map_width(int m) {
    if (m < 0 || m >= map_num) return 0;
    return map[m].xs;
}

int fpm_get_map_height(int m) {
    if (m < 0 || m >= map_num) return 0;
    return map[m].ys;
}

const char* fpm_get_map_name(int m) {
    if (m < 0 || m >= map_num) return "";
    return map[m].name;
}

}

extern "C" {

int fpm_get_sd_x(map_session_data* sd) {
    return sd ? sd->x : 0;
}

int fpm_get_sd_y(map_session_data* sd) {
    return sd ? sd->y : 0;
}

int fpm_get_sd_m(map_session_data* sd) {
    return sd ? sd->m : 0;
}

int fpm_get_bl_x(block_list* bl) {
    return bl ? bl->x : 0;
}

int fpm_get_bl_y(block_list* bl) {
    return bl ? bl->y : 0;
}

int fpm_get_bl_id(block_list* bl) {
    return bl ? bl->id : 0;
}

}

extern "C" map_session_data* fpm_map_id2sd(int account_id) {
    return map_id2sd(account_id); // call rAthena’s native function
}

// State for nearest item search
static block_list* fpm_best_item = nullptr;
static int fpm_best_item_dist = INT_MAX;

// Callback for map_foreachinrange
static int fpm_nearest_item_cb(block_list* bl, va_list ap) {
    map_session_data* sd = va_arg(ap, map_session_data*);
    if (!bl || !sd) return 0;

    // Manhattan distance
    int dx = sd->x - bl->x;
    int dy = sd->y - bl->y;
    int dist = abs(dx) + abs(dy);

    if (dist < fpm_best_item_dist) {
        fpm_best_item_dist = dist;
        fpm_best_item = bl;
    }
    return 0;
}

// Extern C wrappers for FalconPM
extern "C" {

// Find nearest item around a player
block_list* fpm_get_nearest_item(map_session_data* sd, int range) {
    if (!sd) return nullptr;

    block_list* center = (block_list*)sd;
    fpm_best_item = nullptr;
    fpm_best_item_dist = INT_MAX;

    // Iterate over BL_ITEM in range
    map_foreachinrange(fpm_nearest_item_cb, center, (int16)range, BL_ITEM, sd);

    return fpm_best_item;
}

// Make player pick up a floor item
int fpm_pc_loot_item(map_session_data* sd, block_list* item) {
    if (!sd || !item) return 0;
    return pc_takeitem(sd, (flooritem_data*)item); // ✅ cast is required
}

} // extern "C"