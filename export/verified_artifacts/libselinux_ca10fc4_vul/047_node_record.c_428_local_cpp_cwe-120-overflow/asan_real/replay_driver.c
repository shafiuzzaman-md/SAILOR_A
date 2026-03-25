// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// Mirror minimal types used by the harness
typedef struct sepol_handle sepol_handle_t; // opaque

typedef struct sepol_node_s {
    char *addr;
    size_t addr_sz;
    int proto;
    char *mask;
} sepol_node_t;

// Prototype from harness
extern int sepol_node_set_addr_bytes(sepol_handle_t *handle, sepol_node_t *node, const char *addr, size_t addr_sz);

#ifndef SRC_SZ
#define SRC_SZ 8
#endif
#ifndef ADDR_SZ
#define ADDR_SZ 32
#endif

int main() {
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));
    if (!node) return 1;
    node->addr = NULL;
    node->addr_sz = 0;

    char *src = (char *)malloc(SRC_SZ);
    if (!src) return 1;
    { static const unsigned char addr_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, addr_buf_data, (SRC_SZ < sizeof(addr_buf_data)) ? SRC_SZ : sizeof(addr_buf_data)); };

    size_t addr_sz = ADDR_SZ; // concrete and > SRC_SZ to trigger memcpy over-read/write

    sepol_node_set_addr_bytes(NULL, node, src, addr_sz);
    return 0;
}
