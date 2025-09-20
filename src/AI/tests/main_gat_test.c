#include <stdio.h>
#include "peregrine_gat.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <map.gat>\n", argv[0]);
        return 1;
    }

    GatMap* g = gat_load(argv[1]);
    if (!g) return 1;

    printf("Loaded map %s: %dx%d\n", argv[1], g->width, g->height);
    printf("Cell (0,0) walkable? %s\n",
           gat_is_walkable(g, 0, 0) ? "yes" : "no");

    gat_free(g);
    return 0;
}
