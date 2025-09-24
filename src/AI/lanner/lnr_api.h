#ifndef LNR_API_H
#define LNR_API_H

#include "lnr_state.h"
#include "../../infra/plugin_api.h"

// Only declare the initializer here — struct LannerAPI is already in plugin_api.h
#ifdef __cplusplus
extern "C" {
#endif

void lnr_api_init(struct PluginContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // LNR_API_H
