// ------------------------------------------------------
// FalconPM Loader - bridges rAthena symbols to plugins
// Compiles into falconpm_loader.so
// ------------------------------------------------------

#include <dlfcn.h>      // dlopen, dlsym, dlerror
#include <dirent.h>     // scanning plugins/ dir
#include <stdio.h>      // fprintf
#include <string.h>     // strcmp, strstr
#include <stdint.h>     // uint32_t
#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include "plugin_api.h"

// ------------------------------------------------------
// Global API contract (shared with plugins)
// ------------------------------------------------------
static PluginAPI api;

extern "C" {
    PluginAPI* g_plugin_api = &api;   // exported symbol
}

// ------------------------------------------------------
// Bind rAthena symbols dynamically
// ------------------------------------------------------
static void bind_rathena_symbols(void* handle) {
    std::cout << "[FalconPM] Binding rAthena symbols..." << std::endl;

    api.log_info  = (void (*)(const char*, ...)) dlsym(handle, "ShowInfo");
    api.log_error = (void (*)(const char*, ...)) dlsym(handle, "ShowError");
    api.gettick   = (uint32_t (*)(void)) dlsym(handle, "gettick");
    api.add_timer = (int (*)(uint32_t, void (*)(void*), void*)) dlsym(handle, "add_timer");

    if (!api.log_info)   std::cerr << "[FalconPM] Failed to bind ShowInfo" << std::endl;
    if (!api.log_error)  std::cerr << "[FalconPM] Failed to bind ShowError" << std::endl;
    if (!api.gettick)    std::cerr << "[FalconPM] Failed to bind gettick" << std::endl;
    if (!api.add_timer)  std::cerr << "[FalconPM] Failed to bind add_timer" << std::endl;

    std::cout << "[FalconPM] Symbol binding complete." << std::endl;
}

// ------------------------------------------------------
// Load all .so plugins from ./plugins
// ------------------------------------------------------
static void load_plugins() {
    DIR* dir = opendir("./plugins");
    if (!dir) {
        std::cerr << "[FalconPM] Could not open ./plugins directory" << std::endl;
        return;
    }

    int count_loaded = 0;
    struct dirent* ent;

    while ((ent = readdir(dir)) != nullptr) {
        if (strstr(ent->d_name, ".so")) {
            std::string path = std::string("./plugins/") + ent->d_name;
            std::cout << "[FalconPM] Found plugin: " << path << std::endl;

            void* handle = dlopen(path.c_str(), RTLD_NOW);
            if (!handle) {
                std::cerr << "[FalconPM] dlopen failed: " << dlerror() << std::endl;
                continue;
            }

            auto init = (void (*)(PluginAPI*)) dlsym(handle, "plugin_init");
            if (!init) {
                std::cerr << "[FalconPM] dlsym failed in " << ent->d_name
                          << ": " << dlerror() << std::endl;
                dlclose(handle);
                continue;
            }

            init(&api);
            std::cout << "[FalconPM] Initialized " << ent->d_name << std::endl;
            count_loaded++;
        }
    }
    closedir(dir);

    if (count_loaded == 0)
        std::cerr << "[FalconPM] WARNING: No plugins were initialized." << std::endl;
    else
        std::cout << "[FalconPM] " << count_loaded << " plugin(s) initialized." << std::endl;
}

// ------------------------------------------------------
// Entry point (called inside map-server startup)
// ------------------------------------------------------
extern "C" void falconpm_load_plugins(void) {
    std::cout << "[FalconPM] Loader activated." << std::endl;

    void* self = dlopen(nullptr, RTLD_NOW | RTLD_GLOBAL);
    if (!self) {
        std::cerr << "[FalconPM] dlopen(nullptr) failed: " << dlerror() << std::endl;
        return;
    }

    bind_rathena_symbols(self);
    load_plugins();

    std::cout << "[FalconPM] Startup complete, plugins ready." << std::endl;
}
