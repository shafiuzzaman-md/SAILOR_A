#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

// harness_entry is defined in harness/ebitmap.c
int ebitmap_destroy(ebitmap_t *e);

int main() {
    // Allocate the ebitmap object concretely
    ebitmap_t *e = (ebitmap_t *)calloc(1, sizeof(*e));

    // Stack-allocated node to simulate a stale pointer scenario
    ebitmap_node_t local_node;
    // Initialize and constrain traversal to a single iteration
    { static const unsigned char local_node_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&local_node, local_node_data, (sizeof(local_node) < sizeof(local_node_data)) ? sizeof(local_node) : sizeof(local_node_data)); };
    local_node.next = NULL;  // ensure loop ends after first free

    // Set e->node to a non-heap pointer (stack address)
    e->node = &local_node;

    // Direct call path into the vulnerable function via entry
    ebitmap_destroy(e);
    return 0;
}
