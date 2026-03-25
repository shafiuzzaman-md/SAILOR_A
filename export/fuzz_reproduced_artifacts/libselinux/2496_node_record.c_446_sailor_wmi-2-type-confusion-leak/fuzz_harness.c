#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

#ifndef SEPOL_PROTO_IP4
#define SEPOL_PROTO_IP4 4
#endif
#ifndef SEPOL_PROTO_IP6
#define SEPOL_PROTO_IP6 6
#endif

extern int sepol_node_get_mask(sepol_handle_t *handle, const sepol_node_t *node);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 48) return 0;
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    // Concrete buffers, symbolic contents
    const size_t mask_sz = 32;
    char *mask_buf = (char *)malloc(mask_sz);
    { memcpy(mask_buf, fuzz_data + 0, 32); };

    const size_t addr_sz = 16;
    char *addr_buf = (char *)malloc(addr_sz);
    { memcpy(addr_buf, fuzz_data + 32, 16); };

    // Initialize fields
    node->mask = mask_buf;
    node->mask_sz = mask_sz;
    node->addr = addr_buf;
    node->addr_sz = addr_sz;
    node->proto = SEPOL_PROTO_IP4; // valid protocol
    node->con = NULL;

    // UAF setup: free the node, then use it in sepol_node_get_mask
    free(node);

    // This call will dereference the freed node (node->proto/node->mask)
    sepol_node_get_mask(handle, node);
    return 0;
}
