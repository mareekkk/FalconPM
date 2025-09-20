#include <stdio.h>
#include <stdlib.h>
#include "peregrine_gat.h"
#include "peregrine_path.h"

int main(int argc, char** argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <map.gat> sx sy tx ty\n", argv[0]);
        return 1;
    }

    const char* mapfile = argv[1];
    int sx = atoi(argv[2]);
    int sy = atoi(argv[3]);
    int tx = atoi(argv[4]);
    int ty = atoi(argv[5]);

    GatMap* g = gat_load(mapfile);
    if (!g) return 1;

    printf("Start (%d,%d) walkable? %s\n", sx, sy,
           gat_is_walkable(g,sx,sy) ? "yes" : "no");
    printf("Target (%d,%d) walkable? %s\n", tx, ty,
           gat_is_walkable(g,tx,ty) ? "yes" : "no");

    PStepList out;
    if (path_astar(g, sx, sy, tx, ty, &out)) {
        printf("Path found with %d steps:\n", out.count);
        for (int i = 0; i < out.count; i++) {
            printf("  %d: (%d,%d)\n", i, out.steps[i].x, out.steps[i].y);
        }
        free(out.steps);
    } else {
        printf("No path found.\n");
    }

    gat_free(g);
    return 0;
}
