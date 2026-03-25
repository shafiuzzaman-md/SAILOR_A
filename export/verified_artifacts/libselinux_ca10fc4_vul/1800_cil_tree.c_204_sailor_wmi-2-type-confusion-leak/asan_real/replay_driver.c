#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry_func is provided by the harness
int cil_tree_children_destroy(struct cil_tree_node *node);

int main() {
    // Phase 1: allocate a real cil_tree_node
    struct cil_tree_node *node = (struct cil_tree_node *)malloc(sizeof(struct cil_tree_node));
    if (!node) return 0;

    // Make the contents symbolic so the read of cl_head can explore values
    { static const unsigned char cil_tree_node_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(node, cil_tree_node_bytes_data, (sizeof(*node) < sizeof(cil_tree_node_bytes_data)) ? sizeof(*node) : sizeof(cil_tree_node_bytes_data)); };

    // Ensure it's not NULL and pass simple guard expectations
    // flavor can be anything; the harness doesn't branch on it

    // Phase 2: Free the object to create a stale pointer
    free(node);

    // Optional reclaim (type confusion pattern): allocate another chunk of the same size
    // to encourage allocator to reuse the same address, then do not update 'node'.
    void *reclaim = malloc(sizeof(struct cil_tree_node));
    if (reclaim) {
        { static const unsigned char reclaimed_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(reclaim, reclaimed_bytes_data, (sizeof(struct cil_tree_node) < sizeof(reclaimed_bytes_data)) ? sizeof(struct cil_tree_node) : sizeof(reclaimed_bytes_data)); };
    }

    // Phase 3: Use-after-free via stale pointer inside cil_tree_children_destroy
    // This will dereference 'node' (freed) to read cl_head and KLEE should flag .free.err.
    cil_tree_children_destroy(node);

    return 0;
}
