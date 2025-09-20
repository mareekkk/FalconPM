#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MAPS 1024
#define MAX_MAP_NAME 32

int fpm_graph_index(const char *name);
const char* fpm_graph_name(int idx);
void fpm_graph_add_edge(const char *src, const char *dst);
int fpm_graph_has_edge(const char *src, const char *dst);
int fpm_graph_count(void);

#ifdef __cplusplus
}
#endif
