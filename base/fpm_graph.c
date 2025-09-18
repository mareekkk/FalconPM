// ============================================================
// fpm_graph.c
// Builds a graph of maps from portal data and finds routes
// ============================================================

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fpm_portals.c"   // for FPM_Portal + fpm_portals_get

#define MAX_MAPS 1024
#define MAX_ROUTE 128

// Graph adjacency matrix
static char map_names[MAX_MAPS][MAX_MAP_NAME];
static int map_count = 0;
static int adj[MAX_MAPS][MAX_MAPS];

// ------------------------------------------------------------
// Map name -> index
// ------------------------------------------------------------
static int map_index(const char *name) {
    for (int i = 0; i < map_count; i++) {
        if (strcmp(map_names[i], name) == 0) return i;
    }
    if (map_count < MAX_MAPS) {
        strncpy(map_names[map_count], name, MAX_MAP_NAME);
        return map_count++;
    }
    return -1;
}

// ------------------------------------------------------------
// Build graph from loaded portals
// ------------------------------------------------------------
void fpm_graph_build(void) {
    memset(adj, 0, sizeof(adj));
    map_count = 0;

    for (int i = 0; i < fpm_portals_count(); i++) {
        const FPM_Portal *p = fpm_portals_get(i);
        int a = map_index(p->src_map);
        int b = map_index(p->dst_map);
        if (a >= 0 && b >= 0) {
            adj[a][b] = 1; // directed edge
        }
    }
    printf("[FPM] Graph built with %d maps\n", map_count);
}

// ------------------------------------------------------------
// BFS to find a route between maps
// ------------------------------------------------------------
typedef struct {
    char maps[MAX_ROUTE][MAX_MAP_NAME];
    int count;
} FPM_Route;

bool fpm_graph_route(const char *start, const char *goal, FPM_Route *out) {
    int s = map_index(start);
    int g = map_index(goal);
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
        for (int v = 0; v < map_count; v++) {
            if (adj[u][v] && !visited[v]) {
                visited[v] = 1;
                prev[v] = u;
                queue[qt++] = v;
            }
        }
    }

    if (!visited[g]) return false; // no path

    // Reconstruct path
    char stack[MAX_ROUTE][MAX_MAP_NAME];
    int sc = 0;
    for (int v = g; v != -1; v = prev[v]) {
        strncpy(stack[sc++], map_names[v], MAX_MAP_NAME);
    }

    out->count = 0;
    for (int i = sc - 1; i >= 0; i--) {
        strncpy(out->maps[out->count++], stack[i], MAX_MAP_NAME);
    }
    return true;
}
