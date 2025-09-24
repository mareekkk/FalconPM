#include "lnr_api.h"
#include "../../infra/plugin_api.h"

// Forward declarations from lanner.cpp (must match existing LannerAPI)
extern "C" void lanner_tick(void);
extern "C" bool lanner_is_active(void);
extern "C" void lanner_start(struct map_session_data* sd);
extern "C" void lanner_stop(void);

// Local context storage (only accessible through lnr_get_context)
static const PluginContext* g_lanner_context = nullptr;

// Real LannerAPI implementation (static to avoid conflict with stub)
static LannerAPI lanner_api_real = {
    { sizeof(LannerAPI), {FPM_API_VERSION_MAJOR, FPM_API_VERSION_MINOR} },
    lanner_tick,
    lanner_is_active,
    lanner_start,
    lanner_stop
};

// Context accessor function (replaces global variable access)
extern "C" const PluginContext* lnr_get_context(void) {
    return g_lanner_context;
}

// Initialize Lanner API - wire real implementation into context
extern "C" void lnr_api_init(PluginContext* ctx) {
    // Store context locally for accessor function
    g_lanner_context = ctx;
    
    // Replace stub with real implementation
    ctx->lanner = &lanner_api_real;

    if (ctx->log) {
        ctx->log->info("[Lanner] Real API wired (replacing stub) - context accessible via lnr_get_context()");
    }
}