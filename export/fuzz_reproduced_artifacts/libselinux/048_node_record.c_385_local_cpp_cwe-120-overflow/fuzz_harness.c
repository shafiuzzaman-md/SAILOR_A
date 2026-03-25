#include <stdint.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Minimal matching types (must match harness/node_record.c)
typedef struct sepol_handle sepol_handle_t; // opaque

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
} sepol_node_t;

int sepol_node_get_addr_bytes(sepol_handle_t *handle, const sepol_node_t *node);

#ifndef SRC_SZ
#define SRC_SZ 8
#endif

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(*node));
    char *src = (char *)malloc(SRC_SZ);
    if (!src || !node)
        return 0;

    { memcpy(src, fuzz_data + 0, 8); };

    node->addr = src;

    size_t sz;
    { static const unsigned char addr_sz_data[] = {0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&sz, addr_sz_data, (sizeof(sz) < sizeof(addr_sz_data)) ? sizeof(sz) : sizeof(addr_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */
    node->addr_sz = sz;

    sepol_handle_t *handle = NULL;
    sepol_node_get_addr_bytes(handle, node);
    return 0;
}
