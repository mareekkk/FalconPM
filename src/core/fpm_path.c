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
