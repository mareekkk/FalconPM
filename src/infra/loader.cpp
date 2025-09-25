#include <dlfcn.h>
#include <cstdio>
#include <vector>
#include <string>
#include "../infra/plugin_api.h"

extern "C" {

static void* g_base = nullptr;
static std::vector<void*> g_others;

// ------------------------------------------------------------
// Helper: Open a plugin with dlopen and log result
// ------------------------------------------------------------
static void* open_plugin(const char* path, int flags) {
    void* h = dlopen(path, flags);
    if (!h) {
        std::fprintf(stderr, "[FalconPM] dlopen failed for %s: %s\n", path, dlerror());
    } else {
        std::fprintf(stderr, "[FalconPM] loaded %s\n", path);
    }
    return h;
}

// ------------------------------------------------------------
// Helper: Resolve PLUGIN descriptor from a handle
// ------------------------------------------------------------
static PluginDescriptor* get_desc(void* h) {
    if (!h) return nullptr;
    void* sym = dlsym(h, "PLUGIN");
    if (!sym) {
        std::fprintf(stderr, "[FalconPM] missing PLUGIN symbol: %s\n", dlerror());
        return nullptr;
    }
    return reinterpret_cast<PluginDescriptor*>(sym);
}

// Exported by falconpm_base.so
using get_ctx_t = const PluginContext* (*)();

// ------------------------------------------------------------
// Loader Init
// ------------------------------------------------------------
int falconpm_loader_init(void) {
    // Guard against entire loader running multiple times
    static bool loader_already_init = false;
    if (loader_already_init) {
        std::fprintf(stderr, "[FalconPM] loader skipped (already initialized).\n");
        return 1;
    }
    loader_already_init = true;

    // Guard against base init running multiple times
    static bool base_already_init = false;

    // Runtime context (for other plugins)
    const PluginContext* ctx = nullptr;

    // --------------------------------------------------------
    // 1) Initialize FalconPM base once
    // --------------------------------------------------------
    if (!base_already_init) {
        void* base_handle = dlopen("plugins/falconpm_base.so", RTLD_NOW);
        if (!base_handle) {
            std::fprintf(stderr, "[FalconPM] dlopen failed for plugins/falconpm_base.so: %s\n", dlerror());
            return 0;
        }

        g_base = base_handle;
        base_already_init = true;

        // Fetch runtime context for other plugins
        if (auto get_ctx = (get_ctx_t)dlsym(g_base, "falconpm_get_context")) {
            ctx = get_ctx();
        }

        std::fprintf(stderr, "[FalconPM] base initialized.\n");
    } else {
        // If already initialized, just fetch context again
        if (auto get_ctx = (get_ctx_t)dlsym(g_base, "falconpm_get_context")) {
            ctx = get_ctx();
        }
        std::fprintf(stderr, "[FalconPM] base skipped (already initialized).\n");
    }

    // --------------------------------------------------------
    // 2) Load autoroute plugin
    // --------------------------------------------------------
    if (void* h = open_plugin("plugins/autoroute.so", RTLD_NOW | RTLD_LOCAL)) {
        if (auto* desc = get_desc(h)) {
            if (!desc->init(ctx)) {
                std::fprintf(stderr, "[FalconPM] autoroute init failed\n");
            } else {
                g_others.push_back(h);
            }
        }
    }

    // --------------------------------------------------------
    // 3) Load autoattack plugin
    // --------------------------------------------------------
    if (void* h = open_plugin("plugins/autoattack.so", RTLD_NOW | RTLD_LOCAL)) {
        if (auto* desc = get_desc(h)) {
            if (!desc->init(ctx)) {
                std::fprintf(stderr, "[FalconPM] autoattack init failed\n");
            } else {
                g_others.push_back(h);
            }
        }
    }

    return 1;
}

// ------------------------------------------------------------
// Loader Final - unload in reverse order
// ------------------------------------------------------------
void falconpm_loader_final(void) {
    // Shutdown in reverse order for "other" plugins
    for (auto it = g_others.rbegin(); it != g_others.rend(); ++it) {
        if (auto* desc = get_desc(*it)) desc->shutdown();
        if (*it) dlclose(*it);
    }
    g_others.clear();

    // Shutdown base
    if (g_base) {
        if (auto* base = get_desc(g_base)) base->shutdown();
        dlclose(g_base);
        g_base = nullptr;
    }
}

// ------------------------------------------------------------
// rAthena plugin manager entrypoints
// ------------------------------------------------------------
int plugin_init(void)  { return falconpm_loader_init(); }
void plugin_final(void){ falconpm_loader_final(); }

} // extern "C"