/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: fmp_route.c
 *  Description: High-level route planner — sequences multi-map travel using portals and paths
 *
 *  Copyright (C) 2025 Marek
 *  Contact: falconpm@canarybuilds.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

// ============================================================
// fpm_route.c
// Combines fpm_graph + fpm_path for full multi-map routing
// ============================================================

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "fpm_graph.c"
#include "fpm_path.c"

#define MAX_ROUTE 128

typedef struct {
    char maps[MAX_ROUTE][MAX_MAP_NAME];
    int count;
} FPM_Route;

// ------------------------------------------------------------
// BFS between maps (uses graph adjacency)
// ------------------------------------------------------------
bool fpm_route_plan(const char *start, const char *goal, FPM_Route *out) {
    int s = fpm_graph_index(start);
    int g = fpm_graph_index(goal);
    if (s < 0 || g < 0) return false;

    int visited[MAX_MAPS] = {0};
    int prev[MAX_MAPS];
    for (int i = 0; i < MAX_MAPS; i++) prev[i] = -1;

    int queue[MAX_MAPS], qh = 0, qt = 0;
    queue[qt++] = s;
    visited[s] = 1;

    while (qh < qt) {
        int u = queue[qh++];
        if (u == g) break;
        for (int v = 0; v < fpm_graph_count(); v++) {
            if (fpm_graph_has_edge(fpm_graph_name(u), fpm_graph_name(v)) && !visited[v]) {
                visited[v] = 1;
                prev[v] = u;
                queue[qt++] = v;
            }
        }
    }

    if (!visited[g]) return false;

    // Reconstruct path
    char stack[MAX_ROUTE][MAX_MAP_NAME];
    int sc = 0;
    for (int v = g; v != -1; v = prev[v]) {
        strncpy(stack[sc++], fpm_graph_name(v), MAX_MAP_NAME);
    }

    out->count = 0;
    for (int i = sc - 1; i >= 0; i--) {
        strncpy(out->maps[out->count++], stack[i], MAX_MAP_NAME);
    }
    return true;
}

// ------------------------------------------------------------
// Executor stub (would call fpm_pathfind + issue walk cmds)
// ------------------------------------------------------------
void fpm_route_execute(const char *start_map, int start_x, int start_y,
                       const char *goal_map, int goal_x, int goal_y) {
    FPM_Route route;
    if (!fpm_route_plan(start_map, goal_map, &route)) {
        printf("[FPM] No route found from %s to %s\n", start_map, goal_map);
        return;
    }

    printf("[FPM] Planned route (%d hops):\n", route.count);
    for (int i = 0; i < route.count; i++) {
        printf(" -> %s\n", route.maps[i]);
    }

    if (strcmp(start_map, goal_map) == 0) {
        FPM_StepList steps;
        if (fpm_pathfind(start_x, start_y, goal_x, goal_y, &steps)) {
            printf("[FPM] Walking %d steps locally on %s\n", steps.count, start_map);
        }
    } else {
        printf("[FPM] Multi-map routing not yet implemented in executor.\n");
    }
}
