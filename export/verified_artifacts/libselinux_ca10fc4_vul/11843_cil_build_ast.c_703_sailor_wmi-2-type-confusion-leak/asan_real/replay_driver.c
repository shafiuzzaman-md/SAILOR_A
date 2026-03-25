#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Prototype from harness
int cil_gen_perm_nodes(struct cil_db *db, struct cil_tree_node *current_perm, struct cil_tree_node *ast_node, enum cil_flavor flavor, unsigned int *num_perms);

int main() {
    // Opaque db and flavor are unused in harness; pass dummy values
    struct cil_db *db = (struct cil_db*)0;
    enum cil_flavor flavor = (enum cil_flavor)0;

    // num_perms pointer (unused but provide a concrete allocation)
    unsigned int *num_perms = (unsigned int*)malloc(sizeof(unsigned int));
    *num_perms = 0;

    // current_perm must be non-NULL to enter the loop
    struct cil_tree_node *current_perm = (struct cil_tree_node*)malloc(sizeof(struct cil_tree_node));
    // Make its contents symbolic to overapproximate
    { static const unsigned char current_perm_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(current_perm, current_perm_data, (sizeof(*current_perm) < sizeof(current_perm_data)) ? sizeof(*current_perm) : sizeof(current_perm_data)); };

    // Allocate ast_node, then free it to create a stale pointer (UAF / type-confusion setup)
    struct cil_tree_node *ast_node = (struct cil_tree_node*)malloc(sizeof(struct cil_tree_node));
    // Fill memory with symbolic data so the field read is unconstrained
    { static const unsigned char ast_node_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(ast_node, ast_node_bytes_data, (sizeof(*ast_node) < sizeof(ast_node_bytes_data)) ? sizeof(*ast_node) : sizeof(ast_node_bytes_data)); };
    // Free without nullifying references to simulate stale pointer
    free(ast_node);

    // Call entry; cil_gen_perm_nodes will dereference ast_node->cl_head
    // KLEE should detect a free.err on this dereference
    (void)cil_gen_perm_nodes(db, current_perm, ast_node, flavor, num_perms);

    return 0;
}
