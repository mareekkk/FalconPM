// src/AI/peregrine_path.c
// FalconPM - A* pathfinding over GAT maps

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "peregrine_gat.h"

// ---------------------------------------------
// Step structure
// ---------------------------------------------
typedef struct {
    int x, y;
} PStep;

typedef struct {
    PStep* steps;
    int count;
    int capacity;
} PStepList;

void step_list_init(PStepList* list, int cap) {
    list->steps = (PStep*)malloc(sizeof(PStep) * cap);
    list->count = 0;
    list->capacity = cap;
}

void step_list_push(PStepList* list, int x, int y) {
    if (list->count >= list->capacity) return;
    list->steps[list->count].x = x;
    list->steps[list->count].y = y;
    list->count++;
}

void step_list_free(PStepList* list) {
    free(list->steps);
    list->steps = NULL;
    list->count = list->capacity = 0;
}

// ---------------------------------------------
// Node for A*
// ---------------------------------------------
typedef struct Node {
    int x, y;
    int g, f;
    struct Node* parent;
} Node;

// ---------------------------------------------
// Direction vectors (8-way movement)
// ---------------------------------------------
static const int dx[8] = {1,-1,0,0, 1,1,-1,-1};
static const int dy[8] = {0,0,1,-1, 1,-1,1,-1};

// Heuristic: Manhattan
static inline int heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// ---------------------------------------------
// A* pathfinding
// ---------------------------------------------
bool path_astar(const GatMap* g, int sx, int sy, int tx, int ty, PStepList* out) {
    if (!g || !gat_is_walkable(g,sx,sy) || !gat_is_walkable(g,tx,ty)) {
        fprintf(stderr, "[path] invalid start/target\n");
        return false;
    }

    int w = g->width, h = g->height;
    int maxNodes = w * h;

    // visited + cost maps
    uint8_t* visited = (uint8_t*)calloc(maxNodes, 1);
    int* gscore = (int*)malloc(sizeof(int) * maxNodes);
    for (int i=0; i<maxNodes; i++) gscore[i] = INT_MAX;

    // open list (simple array)
    Node* open = (Node*)malloc(sizeof(Node) * maxNodes);
    int openCount = 0;

    // push start
    open[0] = (Node){sx,sy,0,heuristic(sx,sy,tx,ty),NULL};
    gscore[sy*w+sx] = 0;
    openCount = 1;

    Node* found = NULL;

    while (openCount > 0) {
        // find node with lowest f
        int bestIdx = 0;
        for (int i=1;i<openCount;i++) {
            if (open[i].f < open[bestIdx].f) bestIdx = i;
        }
        Node cur = open[bestIdx];
        open[bestIdx] = open[--openCount];

        int idx = cur.y*w + cur.x;
        if (visited[idx]) continue;
        visited[idx] = 1;

        if (cur.x == tx && cur.y == ty) {
            found = (Node*)malloc(sizeof(Node));
            *found = cur;
            break;
        }

        for (int d=0; d<8; d++) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];
            if (!gat_is_walkable(g,nx,ny)) continue;

            int nidx = ny*w + nx;
            int ng = cur.g + 1;
            if (ng < gscore[nidx]) {
                gscore[nidx] = ng;
                int f = ng + heuristic(nx,ny,tx,ty);
                Node nnode = {nx,ny,ng,f, (Node*)malloc(sizeof(Node))};
                *nnode.parent = cur;
                open[openCount++] = nnode;
            }
        }
    }

    if (!found) {
        fprintf(stderr,"[path] no path found\n");
        free(visited); free(gscore); free(open);
        return false;
    }

    // backtrack path
    PStepList rev;
    step_list_init(&rev, w*h);
    Node* n = found;
    while (n) {
        step_list_push(&rev, n->x, n->y);
        n = n->parent;
    }

    // reverse into out
    step_list_init(out, rev.count);
    for (int i = rev.count-1; i>=0; i--) {
        step_list_push(out, rev.steps[i].x, rev.steps[i].y);
    }

    // cleanup
    step_list_free(&rev);
    free(visited); free(gscore); free(open);
    return true;
}
