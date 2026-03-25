#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Prototype from harness
int ebitmap_cpy(ebitmap_t *dst, const ebitmap_t *src);

int main() {
    // Allocate two source bitmaps and a destination bitmap
    ebitmap_t src1, src2, dst;
    memset(&src1, 0, sizeof(src1));
    memset(&src2, 0, sizeof(src2));
    memset(&dst, 0, sizeof(dst));

    // Allocate a shared node (concrete size)
    ebitmap_node_t *node = (ebitmap_node_t *)malloc(sizeof(ebitmap_node_t));
    if (!node) return 0;

    // Make the entire node symbolic; then fix pointer linkage
    { static const unsigned char ebitmap_node_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node, ebitmap_node_data, (sizeof(*node) < sizeof(ebitmap_node_data)) ? sizeof(*node) : sizeof(ebitmap_node_data)); };
    node->next = NULL;  // single-node list

    // Both sources alias the same node
    src1.node = node;
    src2.node = node;
    src1.highbit = 0;
    src2.highbit = 0;

    // Free through one alias without invalidating the other (stale pointer scenario)
    free(src2.node);

    // Now use the stale pointer through src1 in ebitmap_cpy()
    ebitmap_cpy(&dst, &src1);
    return 0;
}
