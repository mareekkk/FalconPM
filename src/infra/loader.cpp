// loader.cpp
// FalconPM loader - bridges rAthena symbols to plugins
// Compiles into falconpm_loader.so / dll

#include <dlfcn.h>       // dlopen/dlsym (Linux/macOS)
#include <dirent.h>      // scanning plugins/ dir
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>

#include "plugin_api.h"

// -----------------------------------------------------------
// Global API struct
// -----------------------------------------------------------
static PluginAPI api;
PluginAPI* g_plugin_api = &api;

// -----------------------------------------------------------
// Symbol binding: find rAthena's functions at runtime
// -----------------------------------------------------------
static void bind_rathena_symbols(void* handle) {
    // Look up ShowInfo, ShowError, gettick, add_timer
    api.log_info  = (void (*)(const char*, ...)) dlsym(handle, "ShowInfo");
    api.log_error = (void (*)(const char*, ...)) dlsym(handle, "ShowError");
    api.gettick   = (uint32_t (*)(void)) dlsym(handle, "gettick");
    api.add_timer = (void (*)(uint32_t, void(*)(void*), void*)) dlsym(handle, "add_timer");

    if (!api.log_info || !api.log_error) {
        fprintf(stderr, "[FalconPM] Failed to bind rAthena symbols.\n");
    }
}

// -----------------------------------------------------------
// Plugin entrypoint typedef
// -----------------------------------------------------------
typedef void (*plugin_init_t)(PluginAPI* api);

// -----------------------------------------------------------
// Load all plugins from ./plugins/ directory
// -----------------------------------------------------------
static void load_plugins() {
    DIR* dir = opendir("plugins");
    if (!dir) {
        std::cerr << "[FalconPM] No plugins directory found.\n";
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        std::string fname = ent->d_name;
        if (fname.size() > 3 && fname.substr(fname.size() - 3) == ".so") {
            std::string fullpath = "plugins/" + fname;
            void* phandle = dlopen(fullpath.c_str(), RTLD_NOW);
            if (!phandle) {
                std::cerr << "[FalconPM] Failed to load " << fname << "\n";
                continue;
            }

            plugin_init_t init = (plugin_init_t)dlsym(phandle, "plugin_init");
            if (init) {
                init(&api); // give plugin the API
                std::cout << "[FalconPM] Loaded plugin: " << fname << "\n";
            }
        }
    }
    closedir(dir);
}

// -----------------------------------------------------------
// Bootstrap called from preload
// -----------------------------------------------------------
extern "C" void falconpm_startup() {
    // Bind to already-loaded rAthena map-server
    void* self = dlopen(nullptr, RTLD_NOW); // handle to main binary
    bind_rathena_symbols(self);

    // Now load all plugins
    load_plugins();

    if (api.log_info) {
        api.log_info("[FalconPM] Startup complete, plugins ready.\n");
    }
}
