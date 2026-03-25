#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int sepol_node_key_extract(sepol_handle_t *handle, const sepol_node_t *node, sepol_node_key_t **key_ptr);

int main() {
    // Concrete allocations for handle and node
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_node_t *node = (sepol_node_t *)calloc(1, sizeof(sepol_node_t));

    // Concrete buffer sizes for node->addr and node->mask
    const size_t ADDR_BUF = 32;   // ensure first memcpy stays in-bounds
    const size_t MASK_BUF = 8;    // intentionally small to trigger OOB read on second memcpy

    // Allocate concrete buffers
    node->addr = (char *)calloc(1, ADDR_BUF);
    node->mask = (char *)calloc(1, MASK_BUF);

    // Make scalar sizes symbolic via locals, then assign to struct
    size_t addr_sz;
    size_t mask_sz;
    { static const unsigned char addr_sz_data[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&addr_sz, addr_sz_data, (sizeof(addr_sz) < sizeof(addr_sz_data)) ? sizeof(addr_sz) : sizeof(addr_sz_data)); };
    { static const unsigned char mask_sz_data[] = {0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&mask_sz, mask_sz_data, (sizeof(mask_sz) < sizeof(mask_sz_data)) ? sizeof(mask_sz) : sizeof(mask_sz_data)); };

    // Constrain sizes
    /* klee_assume removed */
    /* klee_assume removed */
    /* klee_assume removed */   // force overflow on read from node->mask
    /* klee_assume removed */       // reasonable upper bound

    node->addr_sz = addr_sz;
    node->mask_sz = mask_sz;

    // proto not used on the path; set to a concrete value
    node->proto = 0;

    sepol_node_key_t *out_key = NULL;
    sepol_node_key_extract(handle, node, &out_key);
    return 0;
}
