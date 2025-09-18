// ============================================================
// fpm_graph.c
// Maintains graph of maps (nodes + edges) from portal data
// ============================================================

#include <stdio.h>
#include <string.h>

#define MAX_MAPS 1024
#define MAX_MAP_NAME 32

// Adjacency matrix (1 = connected)
static int adj[MAX_MAPS][MAX_MAPS];
static char map_names[MAX_MAPS][MAX_MAP_NAME];
static int map_count = 0;

// ------------------------------------------------------------
// Map name -> index
// ------------------------------------------------------------
int fpm_graph_index(const char *name) {
    for (int i = 0; i < map_count; i++) {
        if (strcmp(map_names[i], name) == 0) return i;
    }
    if (map_count < MAX_MAPS) {
        strncpy(map_names[map_count], name, MAX_MAP_NAME);
        return map_count++;
    }
    return -1;
}

const char* fpm_graph_name(int idx) {
    if (idx < 0 || idx >= map_count) return NULL;
    return map_names[idx];
}

// ------------------------------------------------------------
// Add edge
// ------------------------------------------------------------
void fpm_graph_add_edge(const char *src, const char *dst) {
    int a = fpm_graph_index(src);
    int b = fpm_graph_index(dst);
    if (a >= 0 && b >= 0) {
        adj[a][b] = 1; // directed edge
    }
}

// ------------------------------------------------------------
// Query adjacency
// ------------------------------------------------------------
int fpm_graph_has_edge(const char *src, const char *dst) {
    int a = fpm_graph_index(src);
    int b = fpm_graph_index(dst);
    if (a < 0 || b < 0) return 0;
    return adj[a][b];
}

int fpm_graph_count(void) {
    return map_count;
}
