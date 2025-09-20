// falconpm_bootstrap.cpp
// Lives inside rAthena map-server. Exports unmangled C wrappers that plugins can call.
// Also loads FalconPM loader .so from rAthena/plugins.

#include "pc.hpp"             // declares pc_walktoxy
#include "unit.hpp"           // declares unit_walktoxy
#include "common/showmsg.hpp" // ShowError/ShowInfo
#include <dlfcn.h>

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
