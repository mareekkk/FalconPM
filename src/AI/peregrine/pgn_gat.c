// peregrine_gat.c
// FalconPM - GAT map loader (Gravity / Ragnarok GAT v1.2 spec)
// Reads .gat files and allows walkability checks

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// GAT cell types (simplified)
typedef enum {
    GAT_TYPE_WALKABLE = 0,
    GAT_TYPE_BLOCKED  = 1,
    // later: add water, cliff, etc
} GatCellType;

typedef struct {
    int width;
    int height;
    uint8_t* cells;   // width * height, 0 = walkable, 1 = blocked
} GatMap;

// ----------------------------------------------------
// Helpers: safe little-endian reads
// ----------------------------------------------------
static int32_t read_int32_le(FILE* fp) {
    uint8_t b[4];
    if (fread(b, 1, 4, fp) != 4) return -1;
    return (int32_t)( ((uint32_t)b[0]) |
                      ((uint32_t)b[1] << 8) |
                      ((uint32_t)b[2] << 16) |
                      ((uint32_t)b[3] << 24) );
}

static float read_float_le(FILE* fp) {
    union { uint32_t i; float f; } u;
    uint8_t b[4];
    if (fread(b, 1, 4, fp) != 4) return 0.0f;
    u.i = ((uint32_t)b[0]) |
          ((uint32_t)b[1] << 8) |
          ((uint32_t)b[2] << 16) |
          ((uint32_t)b[3] << 24);
    return u.f;
}

static uint32_t read_uint32_le(FILE* fp) {
    uint8_t b[4];
    if (fread(b, 1, 4, fp) != 4) return 0;
    return ((uint32_t)b[0]) |
           ((uint32_t)b[1] << 8) |
           ((uint32_t)b[2] << 16) |
           ((uint32_t)b[3] << 24);
}

// ----------------------------------------------------
// Load a GAT file into memory
// ----------------------------------------------------
GatMap* gat_load(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "[gat] failed to open %s\n", filename);
        return NULL;
    }

    // Read magic
    char magic[4];
    if (fread(magic, 1, 4, fp) != 4) {
        fprintf(stderr, "[gat] failed to read magic in %s\n", filename);
        fclose(fp);
        return NULL;
    }
    if (strncmp(magic, "GRAT", 4) != 0) {
        fprintf(stderr, "[gat] invalid magic '%c%c%c%c' in %s\n",
                magic[0], magic[1], magic[2], magic[3],
                filename);
        fclose(fp);
        return NULL;
    }

    // version bytes
    uint8_t ver_major, ver_minor;
    if (fread(&ver_major, 1, 1, fp) != 1 ||
        fread(&ver_minor, 1, 1, fp) != 1) {
        fprintf(stderr, "[gat] failed to read version in %s\n", filename);
        fclose(fp);
        return NULL;
    }

    // width & height
    int32_t width = read_int32_le(fp);
    int32_t height = read_int32_le(fp);

    // debug print
    printf("[debug] version=%u.%u width=%d height=%d\n",
           (unsigned)ver_major, (unsigned)ver_minor, width, height);

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "[gat] invalid size %dx%d in %s\n",
                width, height, filename);
        fclose(fp);
        return NULL;
    }

    // allocate GatMap
    GatMap* g = (GatMap*)malloc(sizeof(GatMap));
    if (!g) {
        fprintf(stderr, "[gat] malloc failed for GatMap\n");
        fclose(fp);
        return NULL;
    }
    g->width = width;
    g->height = height;
    g->cells = (uint8_t*)malloc((size_t)width * (size_t)height);
    if (!g->cells) {
        fprintf(stderr, "[gat] malloc failed for cells %dx%d\n", width, height);
        free(g);
        fclose(fp);
        return NULL;
    }

    // Read tiles
    // each tile has: 4 floats (altitudes) + 1 uint32 terrain type
    // Tiles are width*height tiles, in order row by row (y increasing, x increasing)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float h_sw = read_float_le(fp);
            float h_se = read_float_le(fp);
            float h_nw = read_float_le(fp);
            float h_ne = read_float_le(fp);
            (void)h_sw; (void)h_se; (void)h_nw; (void)h_ne;  // not used now

            uint32_t terrain = read_uint32_le(fp);

            // Simplify: terrain == 0 => walkable; else blocked
            // Known values: 0 walkable, 1 blocked, 5 impassable/cliff :contentReference[oaicite:4]{index=4}
            if (terrain == 0) {
                g->cells[y * width + x] = 0;
            } else {
                g->cells[y * width + x] = 1;
            }
        }
    }

    fclose(fp);
    printf("[gat] loaded %s (%dx%d)\n", filename, width, height);
    return g;
}

// ----------------------------------------------------
// Free a GAT map
// ----------------------------------------------------
void gat_free(GatMap* g) {
    if (!g) return;
    free(g->cells);
    free(g);
}

// ----------------------------------------------------
// Query walkability
// ----------------------------------------------------
bool gat_is_walkable(const GatMap* g, int x, int y) {
    if (!g) return false;
    if (x < 0 || y < 0 || x >= g->width || y >= g->height) return false;
    return (g->cells[y * g->width + x] == 0);
}
