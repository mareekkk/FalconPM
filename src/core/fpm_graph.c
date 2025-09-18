/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: fpm_graph.c
 *  Description: Maintains graph of maps (nodes + edges) from portal data
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
