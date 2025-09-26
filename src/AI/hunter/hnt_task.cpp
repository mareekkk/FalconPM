// src/AI/hunter/hnt_task.cpp
#include "hnt_task.h"
#include <cstdio>

// BuffTask: call bootstrap to cast a skill on target
bool BuffTask::execute() {
    const PluginContext* ctx = falconpm_get_context();
    if (!ctx || !ctx->player) return false;

    map_session_data* sd = ctx->player->map_id2sd(target_id);
    if (!sd) return false;

    // Log shim usage for traceability
    std::fprintf(stdout, "[Bootstrap] fpm_unit_skilluse(skill=%d, target=%d)\n",
                 skill_id, target_id);
    std::fflush(stdout);

    fpm_unit_skilluse(sd, skill_id, target_id);
    return true;
}
