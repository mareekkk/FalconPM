#pragma once
#include <stdbool.h>
#include <stdint.h>

struct GatMap {
    int width;
    int height;
    uint8_t* cells;
};

struct GatMap* gat_load(const char* filename);
void           gat_free(struct GatMap* g);
bool           gat_is_walkable(const struct GatMap* g, int x, int y);

typedef struct GatMap GatMap;
