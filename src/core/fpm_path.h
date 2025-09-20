#pragma once
#include "../infra/plugin_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x, y;
} FPM_Step;

typedef struct {
    FPM_Step steps[512];
    int count;
} FPM_StepList;

bool fpm_pathfind(int x1, int y1, int x2, int y2, FPM_StepList *out);

bool fpm_path_execute(struct map_session_data* sd,
                      int x1, int y1, int x2, int y2,
                      const PluginContext* ctx);

#ifdef __cplusplus
}
#endif
