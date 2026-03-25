#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype from harness
extern int sepol_node_set_mask_bytes(sepol_handle_t *handle, sepol_node_t *node, const char *mask, size_t mask_sz);

#ifndef MAX_MASK
#define MAX_MASK 64
#endif
#ifndef IN_SIZE
#define IN_SIZE 8
#endif

int main() {
    // Allocate handle and node concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    // Ensure node->mask points to valid heap memory so free(node->mask) is a valid operation
    const size_t OLD_MASK_SZ = 32; // concrete
    node->mask = (char *)malloc(OLD_MASK_SZ);
    node->mask_sz = OLD_MASK_SZ;
    if (node->mask) {
        { static const unsigned char old_mask_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node->mask, old_mask_bytes_data, (OLD_MASK_SZ < sizeof(old_mask_bytes_data)) ? OLD_MASK_SZ : sizeof(old_mask_bytes_data)); };
        free(node->mask); // create dangling pointer for double-free
        // keep node->mask as dangling pointer to trigger double-free in entry
    }

    // Prepare new mask input buffer with concrete allocation, symbolic contents
    char *in = (char *)malloc(MAX_MASK);
    { static const unsigned char new_mask_input_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(in, new_mask_input_data, (MAX_MASK < sizeof(new_mask_input_data)) ? MAX_MASK : sizeof(new_mask_input_data)); };

    // Symbolic mask size within bounds [1, MAX_MASK]
    size_t sz;
    { static const unsigned char mask_sz_data[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&sz, mask_sz_data, (sizeof(sz) < sizeof(mask_sz_data)) ? sizeof(sz) : sizeof(mask_sz_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    // Direct call to entry
    sepol_node_set_mask_bytes(handle, node, in, sz);
    return 0;
}
