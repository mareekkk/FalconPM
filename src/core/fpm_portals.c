/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: portals.c
 *  Description: Portal handler — manages warp connections between maps
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
// fpm_portals.c
// Loads portal data into memory (from txt or later from rAthena warps)
// ============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PORTALS 2048
#define MAX_MAP_NAME 32

typedef struct {
    char src_map[MAX_MAP_NAME];
    int src_x, src_y;
    char dst_map[MAX_MAP_NAME];
    int dst_x, dst_y;
} FPM_Portal;

static FPM_Portal fpm_portals[MAX_PORTALS];
static int fpm_portal_count = 0;

// ------------------------------------------------------------
// Load portal DB from file: db/fpm_portals.txt
// Format: src_map,src_x,src_y,dst_map,dst_x,dst_y
// ------------------------------------------------------------
void fpm_portals_load(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("[FPM] Failed to open portal DB: %s\n", filename);
        return;
    }

    fpm_portal_count = 0;
    while (!feof(fp) && fpm_portal_count < MAX_PORTALS) {
        FPM_Portal p;
        if (fscanf(fp, "%31[^,],%d,%d,%31[^,],%d,%d\n",
                   p.src_map, &p.src_x, &p.src_y,
                   p.dst_map, &p.dst_x, &p.dst_y) == 6) {
            fpm_portals[fpm_portal_count++] = p;
        }
    }
    fclose(fp);
    printf("[FPM] Loaded %d portals from %s\n", fpm_portal_count, filename);
}

// ------------------------------------------------------------
// Accessors
// ------------------------------------------------------------
int fpm_portals_count(void) {
    return fpm_portal_count;
}

const FPM_Portal* fpm_portals_get(int idx) {
    if (idx < 0 || idx >= fpm_portal_count) return NULL;
    return &fpm_portals[idx];
}
