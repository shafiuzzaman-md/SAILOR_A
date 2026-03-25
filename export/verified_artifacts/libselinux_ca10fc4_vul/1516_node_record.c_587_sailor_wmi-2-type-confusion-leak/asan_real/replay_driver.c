#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int sepol_node_clone(sepol_handle_t *handle, const sepol_node_t *node, sepol_node_t **node_ptr);

int main() {
    // Opaque handle (unused on this path)
    sepol_handle_t *handle = (sepol_handle_t*)calloc(1, 8);

    // Source node
    sepol_node_t *node = (sepol_node_t*)calloc(1, sizeof(sepol_node_t));

    // Set sizes LARGER than allocated buffers to force memcpy to read past source
    node->addr_sz = 64;   // length used by memcpy
    node->mask_sz = 64;   // length used by memcpy

    // Allocate SMALLER buffers than the size fields
    node->addr = (char*)malloc(8);
    node->mask = (char*)malloc(8);

    // Make contents symbolic within allocated size
    { static const unsigned char node_addr_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node->addr, node_addr_bytes_data, (8 < sizeof(node_addr_bytes_data)) ? 8 : sizeof(node_addr_bytes_data)); };
    { static const unsigned char node_mask_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node->mask, node_mask_bytes_data, (8 < sizeof(node_mask_bytes_data)) ? 8 : sizeof(node_mask_bytes_data)); };

    node->proto = 0;
    node->con = NULL;

    sepol_node_t *out = NULL;
    sepol_node_clone(handle, node, &out);
    return 0;
}
