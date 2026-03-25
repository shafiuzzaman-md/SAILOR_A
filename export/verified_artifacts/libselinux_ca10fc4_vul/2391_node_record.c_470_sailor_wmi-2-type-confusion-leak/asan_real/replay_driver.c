// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifndef MASK_ALLOC
#define MASK_ALLOC 32
#endif

/* Minimal type shims matching the harness */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

int sepol_node_get_mask_bytes(sepol_handle_t *handle, const sepol_node_t *node,
               char **buffer, size_t *bsize);

int main() {
    // We don't need a real handle; harness ignores it via ERR macro
    sepol_handle_t *handle = NULL;

    // Allocate and initialize node
    sepol_node_t *node = (sepol_node_t *)malloc(sizeof(sepol_node_t));
    if (!node) return 0;
    memset(node, 0, sizeof(*node));

    // Allocate a small concrete mask buffer
    char *mask = (char *)malloc(MASK_ALLOC);
    if (!mask) return 0;
    { static const unsigned char node_mask_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(mask, node_mask_bytes_data, (MASK_ALLOC < sizeof(node_mask_bytes_data)) ? MASK_ALLOC : sizeof(node_mask_bytes_data)); };

    // Make mask_sz symbolic and constrain to a reasonable range
    size_t msz;
    { static const unsigned char node_mask_sz_data[] = {0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&msz, node_mask_sz_data, (sizeof(msz) < sizeof(node_mask_sz_data)) ? sizeof(msz) : sizeof(node_mask_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */ // avoid huge allocations in memcpy dst

    node->mask = mask;
    node->mask_sz = msz;

    char *out_buf = NULL;
    size_t out_sz = 0;

    // Direct pass-through call to vulnerable path
    sepol_node_get_mask_bytes(handle, node, &out_buf, &out_sz);

    // Cleanup (not strictly necessary for KLEE)
    free(out_buf);
    free(mask);
    free(node);
    return 0;
}
