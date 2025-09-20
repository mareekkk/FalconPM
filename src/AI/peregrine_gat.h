#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int width;
    int height;
    uint8_t* cells;   // width*height grid
} GatMap;

GatMap* gat_load(const char* filename);
void gat_free(GatMap* g);
bool gat_is_walkable(const GatMap* g, int x, int y);
