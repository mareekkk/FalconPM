// src/infra/loader.cpp
#include <dlfcn.h>
#include <cstdio>
#include <vector>
#include <string>

extern "C" {

// Internal state
static void* g_base = nullptr;
static std::vector<void*> g_others;

// Utility to load plugin files
static void* open_plugin(const char* path) {
    void* h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) {
        std::fprintf(stderr, "[FalconPM] dlopen failed for %s: %s\n", path, dlerror());
    } else {
        std::fprintf(stderr, "[FalconPM] loaded %s\n", path);
    }
    return h;
}

// This is the entrypoint rAthena seems to expect
int falconpm_loader_init(void) {
    // Load FalconPM base
    g_base = open_plugin("plugins/falconpm_base.so");
    if (!g_base) {
        return 0;  // failure
    }

    // Load other plugins (optional)
    void* h = open_plugin("plugins/autoroute.so");
    if (h) g_others.push_back(h);

    return 1;  // success
}

// Also export plugin_init for compatibility if needed
int plugin_init(void) {
    return falconpm_loader_init();
}

// Cleanup when unloading
void falconpm_loader_final(void) {
    for (auto* h : g_others) {
        if (h) dlclose(h);
    }
    g_others.clear();

    if (g_base) {
        dlclose(g_base);
        g_base = nullptr;
    }
}

// Also export plugin_final
void plugin_final(void) {
    falconpm_loader_final();
}

} // extern "C"
