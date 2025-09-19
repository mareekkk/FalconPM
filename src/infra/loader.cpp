#include "plugin_api.h"
#include <dlfcn.h>
#include <dirent.h>
#include <cstdarg>
#include <cstdio>
#include <unordered_map>
#include <string>
#include <cstring>  // for strstr

// rAthena functions (resolved at link time)
extern "C" {
    uint32_t gettick(void);
    int add_timer_interval(uint32_t when,
                           int (*cb)(int, uint32_t, int, intptr_t),
                           int id, intptr_t data, uint32_t interval);
    map_session_data* map_id2sd(int aid);
    void clif_displaymessage(int fd, const char* msg);

    int pc_readregistry(map_session_data* sd, int id);
    void pc_setregistry(map_session_data* sd, int id, int val);
}

// ----------------------
// Logging
// ----------------------
static void log_info_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
    va_end(args);
}
static void log_error_impl(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// ----------------------
// Atcommand registration
// ----------------------
using AtCmdHandler = int (*)(int, map_session_data*, const char*, const char*);
extern void atcommand_add(const char*, AtCmdHandler);
static void register_atcommand_impl(const char* name, AtCmdHandler h) {
    atcommand_add(name, h);
}

// ----------------------
// Account variable emulation
// ----------------------
static std::unordered_map<std::string, int> var_map;
static int next_regid = 100000;

static int regid_from_name(const char* name) {
    auto it = var_map.find(name);
    if (it != var_map.end()) return it->second;
    int new_id = next_regid++;
    var_map[name] = new_id;
    return new_id;
}

static int accountvar_get_impl(map_session_data* sd, const char* name) {
    int id = regid_from_name(name);
    return pc_readregistry(sd, id);
}
static void accountvar_set_impl(map_session_data* sd, const char* name, int val) {
    int id = regid_from_name(name);
    pc_setregistry(sd, id, val);
}

// ----------------------
// Global API
// ----------------------
static PluginAPI API = {
    log_info_impl,
    log_error_impl,
    gettick,
    add_timer_interval,
    map_id2sd,
    register_atcommand_impl,
    clif_displaymessage,
    accountvar_get_impl,
    accountvar_set_impl,
};
PluginAPI* g_plugin_api = &API;

// ----------------------
// Plugin discovery
// ----------------------
static void init_plugin(void* so) {
    auto bind = (void(*)(const PluginAPI*))dlsym(so, "plugin_bind_api");
    auto init = (void(*)())dlsym(so, "plugin_init");
    if (!bind || !init) return;
    bind(&API);
    init();
}

static void load_plugins() {
    DIR* dir = opendir("plugins");
    if (!dir) return;

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strstr(ent->d_name, ".so")) {
            char path[256];
            snprintf(path, sizeof(path), "plugins/%s", ent->d_name);
            void* so = dlopen(path, RTLD_NOW);
            if (so) {
                log_info_impl("[FalconPM] Loaded plugin: %s", ent->d_name);
                init_plugin(so);
            } else {
                log_error_impl("[FalconPM] Failed to load %s: %s", ent->d_name, dlerror());
            }
        }
    }
    closedir(dir);
}

// ----------------------
// Entry called by shim (map-server)
// ----------------------
extern "C" void falconpm_loader_init() {
    log_info_impl("[FalconPM] falconpm_loader_init called");
    load_plugins();
}
