#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

/* Entry prototype from harness */
int sepol_node_clone(sepol_handle_t *handle, const sepol_node_t *node);

int main() {
    /* Opaque handle/context allocations (concrete sizes) */
    sepol_handle_t *handle = (sepol_handle_t*)malloc(8);

    /* Phase 1: Create a real sepol_node_t and its context */
    sepol_node_t *n = (sepol_node_t*)calloc(1, sizeof(sepol_node_t));
    n->con = (sepol_context_t*)malloc(16);

    /* Make some bytes symbolic to overapproximate state */
    if (n->con) {
        { static const unsigned char con_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(n->con, con_bytes_data, (16 < sizeof(con_bytes_data)) ? 16 : sizeof(con_bytes_data)); };
    }

    /* Keep a stale reference, then free the object (and its 'con') */
    sepol_node_t *stale = n;
    extern void sepol_node_free(sepol_node_t *node);
    sepol_node_free(n);

    /* Phase 2: Use-after-free — pass stale pointer into clone */
    sepol_node_clone(handle, stale);

    return 0;
}
