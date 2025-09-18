/**
 *  FalconPM - rAthena Plugin Infrastructure
 *  https://github.com/mareekkk/FalconPM
 *
 *  File: fpm_path.c
 *  Description: Pathfinding engine — calculates optimal routes across maps
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
// fpm_path.c
// Local pathfinding inside one map (x,y steps)
// ============================================================

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Step structure
typedef struct {
    int x, y;
} FPM_Step;

#define MAX_STEPS 512

typedef struct {
    FPM_Step steps[MAX_STEPS];
    int count;
} FPM_StepList;

// ------------------------------------------------------------
// Simple straight-line pathfinder (stub)
// Replace with A* using map->getcell for obstacles later
// ------------------------------------------------------------
bool fpm_pathfind(int x1, int y1, int x2, int y2, FPM_StepList *out) {
    out->count = 0;

    int dx = (x2 > x1) ? 1 : -1;
    int dy = (y2 > y1) ? 1 : -1;

    int x = x1, y = y1;
    while (x != x2 || y != y2) {
        if (x != x2) x += dx;
        if (y != y2) y += dy;

        if (out->count < MAX_STEPS) {
            out->steps[out->count].x = x;
            out->steps[out->count].y = y;
            out->count++;
        } else {
            return false; // path too long
        }
    }
    return true;
}
